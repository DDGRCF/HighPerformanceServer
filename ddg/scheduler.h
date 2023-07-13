#ifndef DDG_SCHEDULER_H_
#define DDG_SCHEDULER_H_

#include <list>
#include <memory>
#include <string>
#include <vector>

#include "ddg/fiber.h"
#include "ddg/mutex.h"
#include "ddg/noncopyable.h"
#include "ddg/thread.h"

namespace ddg {

class Scheduler : public NonCopyable {
 public:
  using ptr = std::shared_ptr<Scheduler>();
  using MutexType = Mutex;

 public:
  Scheduler(size_t threads = 1, const std::string& name = "",
            bool use_caller = true);

  virtual ~Scheduler();

  const std::string& getName() const;

  virtual void start();

  virtual void stop();

 public:
  static Scheduler* GetThis();
  static void SetThis(Scheduler* scheduler);

 protected:
  virtual bool stopping();

  virtual void tickle();

  virtual void idle();

  virtual void run();

  bool hasIdleThreads() const;

  bool hasActivateThreads() const;

 protected:
  struct ScheduleTask {
    using ptr = std::shared_ptr<ScheduleTask>;
    using Callback = std::function<void()>;

    Fiber::ptr fiber = nullptr;
    Callback callback = nullptr;
    int thread = -1;

    ScheduleTask(Fiber::ptr f, int t = -1) : fiber(f), thread(t) {}

    ScheduleTask(Fiber::ptr* f, int t = -1) : thread(t) {
      std::swap(*f, fiber);
    }

    ScheduleTask(Callback cb, int t = -1) : callback(cb), thread(t) {}

    ScheduleTask() : thread(-1) {}

    void reset() {
      fiber = nullptr;
      callback = nullptr;
      thread = -1;
    }
  };

 public:
  template <class Task>
  void schedule(Task task, int thread = -1) {
    bool need_tickle = false;
    {
      MutexType::Lock lock(m_mutex);
      need_tickle = scheduleNoLock(task, thread);
    }
    if (need_tickle) {
      tickle();  // 唤醒idle线程
    }
  }

  template <class InputIterator>
  void schedule(InputIterator begin, InputIterator end, int thread = -1) {
    bool need_tickle = false;
    {
      MutexType::Lock lock(m_mutex);
      for (auto it = begin; it != end; it++) {
        need_tickle = scheduleNoLock(*it, thread) || need_tickle;
      }
    }

    if (need_tickle) {
      tickle();  // 唤醒idle线程
    }
  }

 private:
  template <class Task>
  bool scheduleNoLock(Task fc, int thread = 0) {
    ScheduleTask task = ScheduleTask(fc, thread);
    if (task.fiber || task.callback) {
      if (task.fiber && task.fiber->getState() == Fiber::State::HOLD) {
        task.fiber->setState(Fiber::State::READY);
      }
      m_tasks.push_back(task);
    }
    return !m_tasks.empty();
  }

 protected:
  MutexType m_mutex;

  std::string m_name = "UNKNOWN";

  std::vector<Thread::ptr> m_threads;

  std::list<ScheduleTask> m_tasks;

  bool m_use_caller = false;

  std::atomic<size_t> m_active_thread_count{0};

  std::atomic<size_t> m_idle_thread_count{0};

  size_t m_thread_count = 0;

  std::vector<uint64_t> m_thread_ids;

  bool m_shutdown = true;

  bool m_isstart = false;
};

}  // namespace ddg

#endif
