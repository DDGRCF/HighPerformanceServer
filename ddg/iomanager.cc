#include "ddg/iomanager.h"

#include <fcntl.h>
#include <cmath>

#include "ddg/macro.h"

namespace ddg {

static Logger::ptr g_logger = DDG_LOG_ROOT();

IOManager::IOManager(size_t threads, bool use_caller, const std::string& name)
    : Scheduler(threads, use_caller, name) {
  m_epfd = epoll_create(5000);
  DDG_ASSERT(m_epfd > 0);
  int ret = pipe(m_tickle_fds);
  DDG_ASSERT(ret == 0);

  epoll_event ev;
  memset(&ev, 0, sizeof(ev));
  ev.events = EPOLLIN | EPOLLET;
  ev.data.fd = m_tickle_fds[0];  // 接收
  ret = fcntl(m_tickle_fds[0], F_SETFL, O_NONBLOCK);
  DDG_ASSERT(ret == 0);
  ret = epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_tickle_fds[0], &ev);
  DDG_ASSERT(ret == 0);
  contextResize(32);
}

IOManager::~IOManager() {
  if (!Scheduler::isStoped()) {
    stop();
  }

  close(m_epfd);
  close(m_tickle_fds[0]);
  close(m_tickle_fds[1]);

  for (size_t i = 0; i < m_fdContext.size(); i++) {
    if (m_fdContext[i]) {
      delete m_fdContext[i];
    }
  }
}

void IOManager::contextResize(size_t size) {
  m_fdContext.resize(size);

  for (size_t i = 0; i < m_fdContext.size(); i++) {
    if (!m_fdContext[i]) {
      m_fdContext[i] = new FdContext;
      m_fdContext[i]->fd = i;
    }
  }
}

int IOManager::addEvent(int fd, Event event, Callback cb) {
  FdContext* fd_ctx = nullptr;
  RWMutexType::ReadLock lock(m_mutex);
  if (m_fdContext.size() > static_cast<size_t>(fd)) {
    fd_ctx = m_fdContext[fd];
    lock.unlock();
  } else {
    lock.unlock();
    RWMutexType::WriteLock lock2(m_mutex);
    contextResize(1.5 * fd);
    fd_ctx = m_fdContext[fd];
  }

  FdContext::MutexType::Lock lock2(fd_ctx->mutex);

  if (DDG_UNLIKELY(fd_ctx->events & event)) {  // 两个线程操作同一个事件
    DDG_LOG_ERROR(g_logger)
        << "addEvent assert fd = " << fd
        << " event = " << static_cast<EPOLL_EVENTS>(event)
        << " fd_ctx.event = " << static_cast<EPOLL_EVENTS>(fd_ctx->events);
  }

  int op = fd_ctx->events ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;

  epoll_event ev;

  ev.events = EPOLLET | fd_ctx->events | event;
  ev.data.ptr = fd_ctx;

  int ret = epoll_ctl(m_epfd, op, fd, &ev);

  if (ret) {
    DDG_LOG_ERROR(g_logger)
        << "IOManager::addEvent epoll_ctl(" << m_epfd << ", " << op << ", "
        << fd << ", " << static_cast<EPOLL_EVENTS>(ev.events) << ");"
        << "ret = " << ret << " msg = " << strerror(ret)
        << " fd_ctx->events = " << static_cast<EPOLL_EVENTS>(fd_ctx->events);
    return -1;
  }

  m_pendingEventCount++;
  fd_ctx->events = static_cast<Event>(fd_ctx->events | event);
  FdContext::EventContext& event_ctx = fd_ctx->getContext(event);
  DDG_ASSERT(!event_ctx.scheduler && !event_ctx.fiber && !event_ctx.cb);

  event_ctx.scheduler = Scheduler::GetThis();
  if (cb) {
    event_ctx.cb.swap(cb);
  } else {
    event_ctx.fiber = Fiber::GetThis();
    DDG_ASSERT_MSG(event_ctx.fiber->getState() == Fiber::State::EXEC,
                   "state = " << event_ctx.fiber->getState());
  }

  return 0;
}

bool IOManager::delEvent(int fd, Event event) {
  RWMutexType::ReadLock lock(m_mutex);
  if (static_cast<size_t>(fd) >= m_fdContext.size()) {
    return false;
  }

  FdContext* fd_ctx = m_fdContext[fd];
  if (!(fd_ctx->events & event)) {
    return false;
  }

  Event new_events = static_cast<Event>(fd_ctx->events & ~event);
  int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
  epoll_event ev;
  ev.events = EPOLLET | new_events;
  ev.data.ptr = fd_ctx;

  int ret = epoll_ctl(m_epfd, op, fd, &ev);
  if (ret) {
    DDG_LOG_ERROR(g_logger)
        << "IOManager::delEvent(" << m_epfd << ", " << op << ", " << fd << ", "
        << static_cast<EPOLL_EVENTS>(ev.events) << ");" << ret << " = " << ret
        << " msg = " << strerror(ret);
    return false;
  }

  m_pendingEventCount--;
  fd_ctx->events = static_cast<Event>(new_events);

  FdContext::EventContext& event_ctx = fd_ctx->getContext(event);
  fd_ctx->resetContext(event_ctx);
  return true;
}

bool IOManager::cancelEvent(int fd, Event event) {
  RWMutexType::ReadLock lock(m_mutex);

  if (static_cast<size_t>(fd) >= m_fdContext.size()) {
    return false;
  }

  FdContext* fd_ctx = m_fdContext[fd];
  if (DDG_UNLIKELY(!(fd_ctx->events & event))) {
    return false;
  }
  lock.unlock();

  FdContext::MutexType::Lock lock2(fd_ctx->mutex);
  Event new_events = static_cast<Event>(fd_ctx->events & ~event);
  int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;

  epoll_event ev;
  ev.events = op;
  ev.data.ptr = fd_ctx;

  int ret = epoll_ctl(m_epfd, op, fd, &ev);
  if (ret) {
    DDG_LOG_ERROR(g_logger)
        << "IOManager::cancelEvent(" << m_epfd << ", " << op << ", " << fd
        << ", " << static_cast<EPOLL_EVENTS>(ev.events) << ");" << ret << " = "
        << ret << " msg = " << strerror(ret);
    return false;
  }

  fd_ctx->triggerEvent(event);
  m_pendingEventCount--;
  return true;
}

bool IOManager::cancelAll(int fd) {
  RWMutexType::ReadLock lock(m_mutex);

  if (static_cast<size_t>(fd) >= m_fdContext.size()) {
    return false;
  }

  FdContext* fd_ctx = m_fdContext[fd];
  if (!fd_ctx->events) {
    return false;
  }

  lock.unlock();

  FdContext::MutexType::Lock lock2(fd_ctx->mutex);
  int op = EPOLL_CTL_DEL;

  epoll_event ev;
  ev.events = op;
  ev.data.ptr = fd_ctx;

  int ret = epoll_ctl(m_epfd, op, fd, &ev);

  if (ret) {
    DDG_LOG_ERROR(g_logger)
        << "IOManager::cancelEvent epoll_ctl(" << m_epfd << ", " << op << ", "
        << fd << ", " << static_cast<EPOLL_EVENTS>(ev.events) << ");" << ret
        << " = " << ret << " msg = " << strerror(ret);
    return false;
  }

  if (fd_ctx->events & READ) {
    fd_ctx->triggerEvent(READ);
    m_pendingEventCount--;
  }

  if (fd_ctx->events & WRITE) {
    fd_ctx->triggerEvent(WRITE);
    m_pendingEventCount--;
  }

  // fd_ctx = Event::NONE;

  DDG_ASSERT(fd_ctx->events == 0);
  return true;
}

IOManager* IOManager::GetThis() {
  return dynamic_cast<IOManager*>(Scheduler::GetThis());
}

void IOManager::tickle() {
  if (!hasIdleThreads()) {
    return;
  }
  int ret = write(m_tickle_fds[1], "T", 1);
  DDG_ASSERT(ret == 1);
}

bool IOManager::isStoped() {
  return m_pendingEventCount == 0 && Scheduler::isStoped();
}

bool IOManager::isStoped(uint64_t& timeout) {
  timeout = getNextTimer();
  return timeout == ~0ull && isStoped();
}

void IOManager::idle() {
  DDG_LOG_DEBUG(g_logger) << "in idle ...";

  const uint64_t MAX_EVENTS = 256;
  epoll_event* evs = new epoll_event[MAX_EVENTS];
  std::shared_ptr<epoll_event> shared_events(
      evs, [](epoll_event* ptr) { delete[] ptr; });

  while (true) {
    uint64_t next_timeout = 0;
    if (DDG_UNLIKELY(isStoped(next_timeout))) {
      DDG_LOG_DEBUG(g_logger) << "name = " << getName() << " idle stop exit";
      break;
    }

    int ret = 0;
    do {
      static const int MAX_TIMEOUT = 3000;
      if (next_timeout != ~0ull) {
        next_timeout = static_cast<int>(next_timeout) > MAX_TIMEOUT
                           ? MAX_TIMEOUT
                           : next_timeout;
      } else {
        next_timeout = MAX_TIMEOUT;
      }

      ret = epoll_wait(m_epfd, evs, MAX_EVENTS, static_cast<int>(next_timeout));

      if (ret < 0 && errno == EINTR) {
        if (errno == EINTR) {
          DDG_LOG_DEBUG(g_logger)
              << "IOManager::idle epoll_wait has been interrupted";
        }
        continue;
      } else {
        break;
      }
    } while (true);

    std::vector<Callback> cbs;
    listExpiredCallback(cbs);
    if (!cbs.empty()) {
      schedule(cbs.begin(), cbs.end());
      cbs.clear();
    }

    for (int i = 0; i < ret; i++) {
      epoll_event& ev = evs[i];
      if (ev.data.fd == m_tickle_fds[0]) {
        uint8_t dummy[256];
        while (read(m_tickle_fds[0], dummy, sizeof(dummy)) > 0) {
          DDG_LOG_DEBUG(g_logger) << dummy;
        };
        continue;
      }
      FdContext* fd_ctx = static_cast<FdContext*>(ev.data.ptr);
      FdContext::MutexType::Lock lock(fd_ctx->mutex);

      if (ev.events & (EPOLLIN | EPOLLHUP)) {  // EPOLLHUB用于半关闭提醒
        ev.events |= (EPOLLIN | EPOLLHUP) & fd_ctx->events;
      }

      int real_events = NONE;
      if (ev.events & (EPOLLIN | EPOLLHUP)) {
        real_events |= READ;
      }

      if (ev.events & EPOLLOUT) {
        real_events |= WRITE;
      }

      if ((fd_ctx->events & real_events) == NONE) {
        continue;
      }

      int left_evs = fd_ctx->events & ~real_events;
      int op = left_evs ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
      int ret2 = epoll_ctl(m_epfd, op, fd_ctx->fd, &ev);

      if (ret2) {
        DDG_LOG_ERROR(g_logger)
            << "IOManager::idle epoll_ctl(" << m_epfd << ", " << op << ", "
            << fd_ctx->fd << ", " << static_cast<EPOLL_EVENTS>(ev.events)
            << ");" << ret << " = " << ret << " msg = " << strerror(ret);
        continue;
      }

      if (real_events & READ) {
        fd_ctx->triggerEvent(READ);
        m_pendingEventCount--;
      }

      if (real_events & WRITE) {
        fd_ctx->triggerEvent(WRITE);
        m_pendingEventCount--;
      }
    }

    Fiber::ptr cur = Fiber::GetThis();  // 不需要吧，外面自动删除
    auto raw_ptr = cur.get();
    cur.reset();
    raw_ptr->swapOut();
  }
}

void IOManager::onTimerInsertedAtFront() {
  tickle();
}

void IOManager::FdContext::triggerEvent(IOManager::Event event) {
  DDG_ASSERT(events & event);

  events = static_cast<Event>(events & ~event);  // 这里将事件消除
  EventContext& ctx = getContext(event);
  if (ctx.cb) {
    ctx.scheduler->schedule(&ctx.cb);
  } else {
    ctx.scheduler->schedule(&ctx.fiber);
  }
  ctx.scheduler = nullptr;  // TODO: deal events
  return;
}

void IOManager::FdContext::resetContext(EventContext& ctx) {
  ctx.cb = nullptr;
  ctx.fiber.reset();
  ctx.scheduler = nullptr;
}

IOManager::FdContext::EventContext& IOManager::FdContext::getContext(
    Event event) {
  switch (event) {
    case Event::READ:
      return read;
    case Event::WRITE:
      return write;
    default:
      DDG_ASSERT_MSG(false, "getContext");
  }
}

}  // namespace ddg
