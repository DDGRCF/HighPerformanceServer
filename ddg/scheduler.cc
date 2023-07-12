#include "ddg/scheduler.h"

#include "ddg/log.h"
#include "ddg/macro.h"

namespace ddg {

static Logger::ptr g_logger = DDG_LOG_ROOT();

static thread_local Scheduler* t_scheduler = nullptr;

// 如果启动加本线程加入调度，一个线程仅允许一个scheduler
Scheduler::Scheduler(size_t threads, const std::string& name, bool use_caller)
    : m_name(name), m_use_caller(use_caller) {
  DDG_ASSERT(threads > 0);
  if (m_use_caller) {
    DDG_ASSERT(GetThis() == nullptr);
    Scheduler::SetThis(this);
    threads--;

    ddg::Thread::SetName(m_name);

    m_thread_ids.push_back(ddg::GetThreadId());
  }

  m_thread_count = threads;
}

Scheduler::~Scheduler() {
  stop();
  SetThis(nullptr);
}

const std::string& Scheduler::getName() const {
  return m_name;
}

void Scheduler::start() {
  MutexType::Lock lock(m_mutex);
  m_shutdown = false;
  DDG_ASSERT(m_threads.empty());  // 保证非空
  m_threads.resize(m_thread_count);

  for (size_t i = 0; i < m_thread_count; i++) {
    m_threads[i].reset(new Thread(m_name + "_" + std::to_string(i),
                                  std::bind(&Scheduler::run, this)));
    m_thread_ids.push_back(m_threads[i]->getId());
  }
}

void Scheduler::stop() {
  if (stopping()) {
    return;
  }
  m_shutdown = true;

  if (m_use_caller) {
    DDG_ASSERT(this == GetThis());
  } else {
    DDG_ASSERT(this != GetThis());
  }

  for (size_t i = 0; i < m_thread_count; i++) {
    tickle();  // 提醒线程处理
  }

  if (m_use_caller) {
    tickle();
    run();
  }

  // 防止在join的过程中又添加或删除啥导致未释放
  std::vector<Thread::ptr> tmp_threads;
  {
    MutexType::Lock lock(m_mutex);
    tmp_threads.swap(m_threads);
  }

  for (auto&& thread : tmp_threads) {
    thread->join();
  }
}

Scheduler* Scheduler::GetThis() {
  return t_scheduler;
}

void Scheduler::SetThis(Scheduler* scheduler) {
  t_scheduler = scheduler;
}

bool Scheduler::stopping() {
  MutexType::Lock lock(m_mutex);
  return m_shutdown && m_tasks.empty() && !hasActivateThreads();
}

void Scheduler::tickle() {
  DDG_LOG_DEBUG(g_logger) << "scheduler in tickle ...";
}

void Scheduler::idle() {
  DDG_LOG_DEBUG(g_logger) << "scheduler in idle ...";
}

bool Scheduler::hasIdleThreads() const {
  return m_idle_thread_count > 0;
}

bool Scheduler::hasActivateThreads() const {
  return m_active_thread_count > 0;
}

void Scheduler::run() {
  Scheduler::SetThis(this);
  Fiber::GetThis();

  Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle, this)));

  ScheduleTask norm_task;

  while (true) {
    norm_task.reset();
    bool tickle_me = false;
    {
      MutexType::Lock lock(m_mutex);
      auto it = m_tasks.begin();
      while (it != m_tasks.end()) {
        if (it->thread != -1 && it->thread != GetThreadId()) {
          it++;
          tickle_me = true;
          tickle();
          continue;
        }  // 如果thread不等于-1，且调度线程不是本地线程就切换一下

        if (!it->fiber && !it->callback) {
          m_tasks.erase(it++);
          continue;
        }  // 如果是空的任务就，将该任务删除

        if (DDG_UNLIKELY(it->fiber && it->callback)) {  // 如果两个都有就报错
          DDG_ASSERT_MSG(false, "ScheduleTask has fiber and callback both");
        }

        if (it->fiber) {
          DDG_ASSERT(it->fiber->getState() == Fiber::State::READY);
        }  // 如果协程有效，则开始调度

        norm_task = *it;
        m_tasks.erase(it++);
        m_active_thread_count++;
        break;
      }
      tickle_me |= (it != m_tasks.end());  // 如果拿完后，还有线程剩余就退出
    }

    if (tickle_me) {
      tickle();  // 提醒其他线程处理一下
    }

    if (norm_task.callback) {
      norm_task.fiber.reset(new Fiber(norm_task.callback, norm_task.thread));
      norm_task.callback = nullptr;
    }

    if (norm_task.fiber) {
      norm_task.fiber->resume();
      if (norm_task.fiber->getState() == Fiber::State::READY) {
        schedule(norm_task.fiber, norm_task.thread);
      }

      norm_task.reset();
      m_active_thread_count--;
    } else {
      if (idle_fiber->getState() == Fiber::State::TERM) {
        break;
      }
      m_idle_thread_count++;
      idle_fiber->resume();
      m_idle_thread_count--;
    }
  }
}

}  // namespace ddg
