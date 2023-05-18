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
  Scheduler(size_t threads = 1, bool use_caller = true,
            const std::string& name = "");

  virtual ~Scheduler();

  std::string getName() const;

  void SetThis();

  void start();

  void stop();

 public:
  static Scheduler* GetThis();

  static Fiber* GetMainFiber();

 protected:
  virtual bool isStoped();

  virtual void tickle();

  virtual void idle();

  bool hasIdleThreads() { return m_idleThreadCount > 0; }

  void run();

 private:
  struct FiberAndThread {
    using ptr = std::shared_ptr<FiberAndThread>;

    using Callback = std::function<void()>;

    Fiber::ptr fiber;
    Callback cb;
    u_int64_t thread;

    FiberAndThread(Fiber::ptr f, uint64_t thr = 0) : fiber(f), thread(thr) {}

    FiberAndThread(Fiber::ptr* f, uint64_t thr = 0) : thread(thr) {
      fiber.swap(*f);
    }

    FiberAndThread(Callback f, uint64_t thr = 0) : cb(f), thread(thr) {}

    FiberAndThread(Callback* f, uint64_t thr = 0) : thread(thr) { cb.swap(*f); }

    FiberAndThread() : thread(0) {}

    void reset() {
      fiber = nullptr;
      cb = nullptr;
      thread = 0;
    }

   private:
  };

 public:
  template <class FiberOrCb>
  void schedule(FiberOrCb fc, uint64_t thread = 0) {
    bool need_tickle = false;
    {
      MutexType::Lock lock(m_mutex);
      need_tickle = scheduleNoLock(fc, thread);
    }
    if (need_tickle) {
      tickle();
    }
  }

  template <class InputIterator>
  void schedule(InputIterator begin, InputIterator end) {
    bool need_tickle = false;
    {
      MutexType::Lock lock(m_mutex);
      for (auto it = begin; it != end; it++) {
        need_tickle = scheduleNoLock(&*it) || need_tickle;
      }
    }

    if (need_tickle) {
      tickle();
    }
  }

 private:
  template <class FiberOrCb>
  bool scheduleNoLock(FiberOrCb fc, int thread) {
    FiberAndThread::ptr ft = std::make_shared<FiberAndThread>(fc, thread);
    if (ft->fiber || ft->cb) {
      m_fibers.push_back(ft);
    }
    return !m_fibers.empty();
  }

  bool scheduleNoLock(FiberAndThread::ptr ft) {
    if (ft->fiber || ft->cb) {
      m_fibers.push_back(ft);
    }
    return !m_fibers.empty();
  }

  void schedule(FiberAndThread::ptr ft) {
    bool need_tickle = false;
    {
      MutexType::Lock lock(m_mutex);
      need_tickle = scheduleNoLock(ft);
    }
    if (need_tickle) {
      tickle();
    }
  }

 private:
  MutexType m_mutex;

  std::string m_name = "UNKNOWN";
  std::vector<Thread::ptr> m_threads;
  std::list<FiberAndThread::ptr> m_fibers;
  Fiber::ptr m_rootFiber;

 protected:
  std::vector<uint64_t> m_threadIds;

  size_t m_threadCount = 0;

  size_t m_activeThreadCount;
  size_t m_idleThreadCount;
  bool m_stopping = true;   // 是否停止
  bool m_autoStop = false;  // 是否自动停止
  uint64_t m_rootThread = 0;
};

}  // namespace ddg

#endif
