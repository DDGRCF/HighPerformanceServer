#include "ddg/iomanager.h"

#include <fcntl.h>
#include <cmath>

#include "ddg/config.h"
#include "ddg/hook.h"
#include "ddg/macro.h"
#include "ddg/mutex.h"

namespace ddg {

static Logger::ptr g_logger = DDG_LOG_ROOT();

static ConfigVar<int>::ptr g_iomanager_epoll_size =
    Config::Lookup<int>("iomanager.epoll_size", 5000,
                        "epoll size");  // deprecated

static ConfigVar<int>::ptr g_iomanager_max_events =
    Config::Lookup<int>("iomanager.max_events", 500, "max events(ms)");

static ConfigVar<time_t>::ptr g_iomanager_max_timeout =
    Config::Lookup<time_t>("iomanager.max_timeout", 3000, "max timeout(ms)");

std::ostream& operator<<(std::ostream& os, const EpollCtlOp& op) {
  switch (static_cast<int>(op)) {
#define XX(ctl) \
  case ctl:     \
    return os << #ctl;
    XX(EPOLL_CTL_ADD);
    XX(EPOLL_CTL_MOD);
    XX(EPOLL_CTL_DEL);
#undef XX
    default:
      return os << static_cast<int>(op);
  }
}

std::ostream& operator<<(std::ostream& os, EPOLL_EVENTS events) {
  if (!events) {
    return os << "0";
  }
  bool first = true;
#define XX(E)       \
  if (events & E) { \
    if (!first) {   \
      os << "|";    \
    }               \
    os << #E;       \
    first = false;  \
  }
  XX(EPOLLIN);
  XX(EPOLLPRI);
  XX(EPOLLOUT);
  XX(EPOLLRDNORM);
  XX(EPOLLRDBAND);
  XX(EPOLLWRNORM);
  XX(EPOLLWRBAND);
  XX(EPOLLMSG);
  XX(EPOLLERR);
  XX(EPOLLHUP);
  XX(EPOLLRDHUP);
  XX(EPOLLONESHOT);
  XX(EPOLLET);
#undef XX
  return os;
}

std::string IOManager::Event::ToString(IOManager::Event::Type type) {
  switch (type) {
#define XX(type)                     \
  case IOManager::Event::Type::type: \
    return #type;
    XX(NONE)
    XX(READ)
    XX(WRITE)
#undef XX
    default:
      return "UNKNOW";
  }
}

IOManager::Event::Type IOManager::Event::FromString(const std::string& name) {
#define XX(type)                   \
  if (#type == name) {             \
    return IOManager::Event::type; \
  }
  XX(NONE)
  XX(READ)
  XX(WRITE)
#undef XX
  return IOManager::Event::UNKNOW;
}

IOManager::IOManager(size_t threads, const std::string& name, bool use_caller,
                     size_t epoll_size)
    : Scheduler(threads, name, use_caller) {
  m_epfd = epoll_create(epoll_size ? epoll_size
                                   : g_iomanager_epoll_size->getValue());
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

  eventResize(32);
  set_hook_enable(true);
}

IOManager::~IOManager() {
  if (!Scheduler::stopping()) {
    stop();
  }

  close(m_tickle_fds[0]);
  close(m_tickle_fds[1]);
  close(m_epfd);

  for (size_t i = 0; i < m_fd_events.size(); i++) {
    if (m_fd_events[i]) {
      delete m_fd_events[i];
    }
  }
}

void IOManager::start() {
  MutexType::Lock lock(Scheduler::m_mutex);
  if (m_isstart) {
    return;
  }
  m_shutdown = false;
  DDG_ASSERT(m_threads.empty());  // 保证非空
  m_threads.resize(m_thread_count);

  for (size_t i = 0; i < m_thread_count; i++) {
    m_threads[i].reset(new Thread(m_name + "_" + std::to_string(i), [this] {
      set_hook_enable(true);
      run();
    }));
    m_thread_ids.push_back(m_threads[i]->getId());
  }
  m_isstart = true;
}

bool IOManager::hasPendingEvents() const {
  return m_pending_event_count > 0;
}

bool IOManager::addEvent(int fd, Event::Type event, Callback cb) {
  FdEvent* fd_event = nullptr;
  RWMutexType::ReadLock lock(m_mutex);

  // 每次获得fd，fd是从最低位开始增长的，如果不足就扩大context
  if (m_fd_events.size() > static_cast<size_t>(fd)) {
    fd_event = m_fd_events[fd];
    lock.unlock();
  } else {
    lock.unlock();
    RWMutexType::WriteLock lock2(m_mutex);
    eventResize(1.5 * fd);
    fd_event = m_fd_events[fd];
  }

  FdEvent::MutexType::Lock lock2(fd_event->mutex);

  // fd_event 上已经有该事件了 两个线程操作同一个事件
  if (DDG_UNLIKELY(fd_event->events & event)) {
    DDG_LOG_ERROR(g_logger)
        << "IOManager::addEvent assert fd = " << fd
        << " event = " << Event::ToString(event)
        << " fd_event.events = " << Event::ToString(fd_event->events);
    return false;
  }

  // 如果没有事件就添加事件，没有就修改
  int op = fd_event->events ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;

  epoll_event ev;

  // 重新赋值事件，确保边缘触发
  ev.events = EPOLLET | fd_event->events | event;
  ev.data.ptr = fd_event;

  int ret = epoll_ctl(m_epfd, op, fd, &ev);
  if (ret) {
    DDG_LOG_ERROR(g_logger)
        << "IOManager::addEvent epoll_ctl(" << m_epfd << ", "
        << static_cast<EpollCtlOp>(op) << ", " << fd << ", "
        << static_cast<EPOLL_EVENTS>(ev.events) << ");"
        << "ret = " << ret << " msg = " << strerror(ret)
        << " fd_event->eventss = " << Event::ToString(fd_event->events);
    return false;
  }

  m_pending_event_count++;
  fd_event->events = static_cast<Event::Type>(fd_event->events | event);
  FdEvent::EventContext& event_ctx = fd_event->getContext(event);
  DDG_ASSERT(!event_ctx.scheduler && !event_ctx.fiber && !event_ctx.callback);
  event_ctx.scheduler = Scheduler::GetThis();  // 调度器是当前线程的调度器

  if (cb) {
    event_ctx.callback = cb;
  } else {
    event_ctx.fiber = Fiber::GetThis();  // 协程是当前运行的协程
    DDG_ASSERT_MSG(
        event_ctx.fiber->getState() == Fiber::State::RUNNING,
        "state = " << Fiber::State::ToString(event_ctx.fiber->getState()));
  }

  return true;
}

bool IOManager::delEvent(int fd, Event::Type event) {
  RWMutexType::ReadLock lock(m_mutex);
  if (static_cast<size_t>(fd) >= m_fd_events.size()) {
    return false;
  }

  FdEvent* fd_event = m_fd_events[fd];
  if (!(fd_event->events & event)) {
    return false;
  }

  // 把事件的位置为0，其他位置不变
  Event::Type new_events = static_cast<Event::Type>(fd_event->events & ~event);

  int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
  epoll_event ev;
  ev.events = EPOLLET | new_events;
  ev.data.ptr = fd_event;

  int ret = epoll_ctl(m_epfd, op, fd, &ev);
  if (ret) {
    DDG_LOG_ERROR(g_logger)
        << "IOManager::delEvent(" << m_epfd << ", "
        << static_cast<EpollCtlOp>(op) << ", " << fd << ", "
        << static_cast<EPOLL_EVENTS>(ev.events) << ");" << ret << " = " << ret
        << " msg = " << strerror(ret);
    return false;
  }

  m_pending_event_count--;
  fd_event->events = new_events;

  FdEvent::EventContext& event_ctx = fd_event->getContext(event);
  fd_event->resetContext(event_ctx);
  return true;
}

bool IOManager::cancelEvent(int fd, Event::Type event) {
  RWMutexType::ReadLock lock(m_mutex);

  if (static_cast<size_t>(fd) >= m_fd_events.size()) {
    return false;
  }

  FdEvent* fd_event = m_fd_events[fd];
  if (!(fd_event->events & event)) {
    return false;
  }
  lock.unlock();

  FdEvent::MutexType::Lock lock2(fd_event->mutex);
  Event::Type new_events = static_cast<Event::Type>(fd_event->events & ~event);
  int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;

  epoll_event ev;
  ev.events = op;
  ev.data.ptr = fd_event;

  int ret = epoll_ctl(m_epfd, op, fd, &ev);
  if (ret) {
    DDG_LOG_ERROR(g_logger)
        << "IOManager::cancelEvent(" << m_epfd << ", "
        << static_cast<EpollCtlOp>(op) << ", " << fd << ", "
        << static_cast<EPOLL_EVENTS>(ev.events) << ");" << ret << " = " << ret
        << " msg = " << strerror(ret);
    return false;
  }

  // 如果取消事件，就会触发事件
  fd_event->triggerEvent(event);
  m_pending_event_count--;
  return true;
}

bool IOManager::cancelAll(int fd) {
  RWMutexType::ReadLock lock(m_mutex);

  if (static_cast<size_t>(fd) >= m_fd_events.size()) {
    return false;
  }

  FdEvent* fd_event = m_fd_events[fd];
  if (!fd_event->events) {
    return false;
  }
  lock.unlock();

  FdEvent::MutexType::Lock lock2(fd_event->mutex);
  int op = EPOLL_CTL_DEL;

  epoll_event ev;
  ev.events = op;
  ev.data.ptr = fd_event;

  int ret = epoll_ctl(m_epfd, op, fd, &ev);

  if (ret) {
    DDG_LOG_ERROR(g_logger)
        << "IOManager::cancelEvent epoll_ctl(" << m_epfd << ", "
        << static_cast<EpollCtlOp>(op) << ", " << fd << ", "
        << static_cast<EPOLL_EVENTS>(ev.events) << ");" << ret << " = " << ret
        << " msg = " << strerror(ret);
    return false;
  }

  if (fd_event->events & Event::READ) {
    fd_event->triggerEvent(Event::READ);
    m_pending_event_count--;
  }

  if (fd_event->events & Event::WRITE) {
    fd_event->triggerEvent(Event::WRITE);
    m_pending_event_count--;
  }

  // 只能有两种事件
  DDG_ASSERT(fd_event->events == 0);
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
  if (DDG_UNLIKELY(ret != 1)) {
    DDG_ASSERT_MSG(false, "IOManager::tickle write");
  }
}

bool IOManager::stopping() {
  time_t timeout = 0;
  return stopping(timeout);
}

bool IOManager::stopping(time_t& timeout) {
  timeout = getNextTimer();
  return timeout == -1 && Scheduler::stopping() && !hasPendingEvents();
}

void IOManager::idle() {
  static const time_t MAX_TIMEOUT = g_iomanager_max_timeout->getValue();
  static const int MAX_EVENTS = g_iomanager_max_events->getValue();

  epoll_event* evs = new epoll_event[MAX_EVENTS];
  std::shared_ptr<epoll_event> shared_events(
      evs, [](epoll_event* ptr) { delete[] ptr; });

  while (true) {
    time_t next_timeout = -1;
    if (DDG_UNLIKELY(stopping(next_timeout))) {
      break;
    }

    int ret = 0;
    do {
      if (next_timeout != -1) {
        next_timeout = next_timeout > MAX_TIMEOUT ? MAX_TIMEOUT : next_timeout;
      } else {
        next_timeout = MAX_TIMEOUT;
      }

      ret = epoll_wait(m_epfd, evs, MAX_EVENTS, static_cast<int>(next_timeout));

      if (ret < 0 && errno == EINTR) {
        if (errno == EINTR) {
          DDG_LOG_WARN(g_logger)
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
        uint8_t dummy[MAX_EVENTS];
        while (read(m_tickle_fds[0], dummy, sizeof(dummy)) > 0)
          ;
        continue;
      }
      FdEvent* fd_event = static_cast<FdEvent*>(ev.data.ptr);
      FdEvent::MutexType::Lock lock(fd_event->mutex);

      // EPOLLERR：容易出现错误
      // EPOLLHUB：套接字对端已经关闭了
      if (ev.events & (EPOLLERR | EPOLLHUP)) {
        ev.events |= (EPOLLIN | EPOLLOUT) & fd_event->events;
      }

      int real_events = Event::NONE;
      if (ev.events & EPOLLIN) {
        real_events |= Event::READ;
      }

      if (ev.events & EPOLLOUT) {
        real_events |= Event::WRITE;
      }

      if ((fd_event->events & real_events) == Event::NONE) {
        continue;
      }

      int left_evs = (fd_event->events & ~real_events);
      int op = left_evs ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
      ev.events = EPOLLET | left_evs;

      ret = epoll_ctl(m_epfd, op, fd_event->fd, &ev);

      if (ret) {
        DDG_LOG_ERROR(g_logger)
            << "IOManager::idle epoll_ctl(" << m_epfd << ", "
            << static_cast<EpollCtlOp>(op) << ", " << fd_event->fd << ", "
            << static_cast<EPOLL_EVENTS>(ev.events) << ");" << ret << " = "
            << ret << " msg = " << strerror(ret);
        continue;
      }

      if (real_events & Event::READ) {
        fd_event->triggerEvent(Event::READ);
        m_pending_event_count--;
      }

      if (real_events & Event::WRITE) {
        fd_event->triggerEvent(Event::WRITE);
        m_pending_event_count--;
      }
    }

    Fiber::ptr cur = Fiber::GetThis();
    auto raw_ptr = cur.get();
    cur.reset();
    raw_ptr->yield();
  }
}

void IOManager::onTimerInsertedAtFront() {
  tickle();
}

void IOManager::FdEvent::triggerEvent(IOManager::Event::Type event) {
  DDG_ASSERT(events & event);

  events = static_cast<Event::Type>(events & ~event);  // 这里将事件消除
  EventContext& ctx = getContext(event);
  if (ctx.callback) {
    ctx.scheduler->schedule(ctx.callback);
  } else {
    ctx.scheduler->schedule(ctx.fiber);
  }
  resetContext(ctx);
}

void IOManager::FdEvent::resetContext(EventContext& ctx) {
  ctx.callback = nullptr;
  ctx.fiber.reset();
  ctx.scheduler = nullptr;
}

IOManager::FdEvent::EventContext& IOManager::FdEvent::getContext(
    Event::Type event) {
  switch (event) {
    case Event::READ:
      return read;
    case Event::WRITE:
      return write;
    default:
      DDG_ASSERT_MSG(false, "IOManager::FdEvent::getContext");
  }
}

void IOManager::eventResize(size_t size) {
  m_fd_events.resize(size);

  for (size_t i = 0; i < m_fd_events.size(); i++) {
    if (!m_fd_events[i]) {
      m_fd_events[i] = new FdEvent;
      m_fd_events[i]->fd = i;
    }
  }
}

}  // namespace ddg
