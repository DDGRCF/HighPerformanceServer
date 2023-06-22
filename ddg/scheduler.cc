#include "ddg/scheduler.h"

#include "ddg/hook.h"
#include "ddg/log.h"
#include "macro.h"

namespace ddg {

static Logger::ptr g_logger = DDG_LOG_ROOT();

static thread_local Scheduler* t_scheduler = nullptr;

static thread_local Fiber* t_scheduler_fiber = nullptr;

Scheduler::Scheduler(size_t threads, bool use_caller, const std::string& name)
    : m_name(name) {
  DDG_ASSERT(threads > 0);
  if (use_caller) {
    Fiber::GetThis();
    --threads;  // 本线程参与 TODO:

    DDG_ASSERT(GetThis() == nullptr);
    t_scheduler = this;

    m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this), 0, true));
    ddg::Thread::SetName(m_name);

    t_scheduler_fiber = m_rootFiber.get();

    m_rootThread = ddg::GetThreadId();
    m_threadIds.push_back(m_rootThread);
  } else {
    m_rootThread = 0;
  }

  m_threadCount = threads;
}

Scheduler::~Scheduler() {}

std::string Scheduler::getName() const {
  return m_name;
}

void Scheduler::tickle() {
  DDG_LOG_DEBUG(g_logger) << "in tickle ...";
}

void Scheduler::idle() {
  DDG_LOG_DEBUG(g_logger) << "in idle ...";
}

void Scheduler::start() {
  MutexType::Lock lock(m_mutex);
  m_stopping = false;
  DDG_ASSERT(m_threads.empty());  // 保证非空
  m_threads.resize(m_threadCount);
  for (size_t i = 0; i < m_threadCount; i++) {
    m_threads[i].reset(new Thread(m_name + "_" + std::to_string(i),
                                  std::bind(&Scheduler::run, this)));
    m_threadIds.push_back(m_threads[i]->getId());
  }
}

void Scheduler::stop() {
  m_autoStop = true;
  if (isStoped()) {
    return;
  }

  if (m_rootFiber && (m_rootFiber->getState() == Fiber::State::TERM ||
                      m_rootFiber->getState() == Fiber::State::EXCEPT)) {
    DDG_LOG_DEBUG(g_logger) << this << " stopped";
    m_stopping = true;
    if (isStoped()) {
      return;
    }
  }

  if (m_rootThread != 0) {
    DDG_ASSERT(GetThis() == this);
  } else {
    DDG_ASSERT(GetThis() != this);
  }

  m_stopping = true;

  for (size_t i = 0; i < m_threadCount; ++i) {
    tickle();
  }

  if (m_rootFiber) {
    tickle();
  }

  if (m_rootFiber) {
    m_rootFiber->setState(Fiber::State::EXEC);

    if (!isStoped()) {
      m_rootFiber
          ->call();  // 这里有个问题，就是无法在当前协程里面再添加协程，因为不是主要是因为不是主携程调度，swapIn的时候切换的是两个协程
    }
  }

  std::vector<Thread::ptr> thrs;
  {
    MutexType::Lock lock(m_mutex);
    thrs.swap(m_threads);
  }

  for (auto& i : thrs) {
    i->join();
  }
}

bool Scheduler::isStoped() {
  return m_autoStop && m_stopping && m_fibers.empty() &&
         m_activeThreadCount == 0;
}

Scheduler* Scheduler::GetThis() {
  return t_scheduler;
}

void Scheduler::SetThis() {
  t_scheduler = this;
}

Fiber* Scheduler::GetMainFiber() {
  return t_scheduler_fiber;
}

void Scheduler::run() {
  DDG_LOG_DEBUG(g_logger) << "into run ...";
  SetThis();
  if (ddg::GetThreadId() != m_rootThread) {
    t_scheduler_fiber = Fiber::GetThis().get();
  }

  FiberAndThread::ptr ft;
  auto idle_ft = std::make_shared<FiberAndThread>(
      std::make_shared<Fiber>(std::bind(&Scheduler::idle, this)));

  while (true) {
    bool is_active = false;
    bool tickle_me = false;
    {
      MutexType::Lock lock(m_mutex);
      for (auto it = m_fibers.begin(); it != m_fibers.end(); it++) {
#define itt (*it)
        if (itt->thread != 0 && itt->thread == GetThreadId()) {
          tickle_me = true;
          continue;
        }

        if (itt->fiber) {
          auto fiber = itt->fiber;
          auto state = fiber->getState();
          bool is_skip = false;
          switch (state) {
            case Fiber::State::INIT:
            case Fiber::State::HOLD:  // 预备
              fiber->setState(Fiber::State::READY);
              is_skip = true;
              break;
            case Fiber::State::EXEC:  // 挂起
              is_skip = true;
              break;
            case Fiber::State::READY:
              is_skip = false;
              break;
            default:
              m_fibers.erase(it++);
              is_skip = true;
          }

          if (is_skip) {
            continue;
          }
        }

        ft = itt;
        m_fibers.erase(it++);
        m_activeThreadCount++;
        is_active = true;
        tickle_me = is_active;
        break;
#undef itt
      }
    }

    if (tickle_me) {
      tickle();
    }

    if (is_active && (ft->fiber || ft->cb)) {
      Fiber::ptr fiber;
      if (ft->fiber) {
        fiber = ft->fiber;
      } else if (ft->cb) {
        fiber = std::make_shared<Fiber>(ft->cb);
        ft->cb = nullptr;
        ft->fiber = fiber;
      }

      fiber->setState(Fiber::State::Type::READY);
      fiber->swapIn();
      m_activeThreadCount--;
      auto state = fiber->getState();  // 如果是调用当前携程的话没法释放
      // DDG_LOG_DEBUG(g_logger)
      // << "come here "
      // << "Fiber State: " << Fiber::State::ToString(fiber->getState());
      if (state == Fiber::State::READY) {
        schedule(ft);
      } else if (state != Fiber::State::TERM && state != Fiber::State::EXCEPT) {
        ft->fiber->setState(Fiber::State::HOLD);
      }

      ft.reset();  // 重新设置智能指针
    } else {
      if (is_active) {
        m_activeThreadCount--;
        continue;
      }
      if (idle_ft) {
        if (idle_ft->fiber) {
          auto fiber = idle_ft->fiber;
          m_idleThreadCount++;
          fiber->swapIn();
          m_idleThreadCount--;
          if (fiber->getState() == Fiber::State::EXEC) {
            fiber->setState(Fiber::State::HOLD);
          } else if (fiber->getState() == Fiber::State::TERM ||
                     fiber->getState() == Fiber::State::EXCEPT) {
            idle_ft.reset();
          }
        }
      } else {
        DDG_LOG_DEBUG(g_logger) << "idle_ft count: " << idle_ft.use_count();
        break;
      }
    }
  }
}

}  // namespace ddg
