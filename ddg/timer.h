#ifndef DDG_TIMER_H_
#define DDG_TIMER_H_

#include <memory>

#include "ddg/log.h"
#include "ddg/mutex.h"
#include "ddg/noncopyable.h"

namespace ddg {

class TimerManager;

using Callback = std::function<void()>;

class Timer : public std::enable_shared_from_this<Timer> {
 public:
  friend TimerManager;

  using ptr = std::shared_ptr<Timer>;

  bool cancel();

  bool refresh();

  bool reset(uint64_t ms, bool from_now = true);

  Timer(uint64_t ms, Callback cb, bool recusring, TimerManager* manager);

  Timer(uint64_t ms, Callback cb, bool recusring, TimerManager& manager);

 private:
  Timer(uint64_t next);

 private:
  struct Comparator {
    bool operator()(const Timer::ptr& lhs, const Timer::ptr& rhs) const;
  };

 private:
  uint64_t m_ms = 0;
  Callback m_cb;
  bool m_recursing = false;

  uint64_t m_next = 0;

  TimerManager* m_manager = nullptr;
};

class TimerManager : public NonCopyable {
 public:
  friend Timer;
  using RWMutexType = RWMutex;

 public:
  Timer::ptr addTimer(uint64_t ms, Callback cb, bool recursing = false);

 private:
  void addTimer(Timer::ptr val);

  bool addTimerNoLock(Timer::ptr val);

  bool detectClockRollover(uint64_t now_ms);

 public:
  TimerManager();
  virtual ~TimerManager();

  Timer::ptr addConditionTimer(uint64_t ms, Callback cb,
                               std::weak_ptr<void> weak_cond,
                               bool recursing = false);

  uint64_t getNextTimer();

  void listExpiredCallback(std::vector<Callback>& cb);

  bool hasTimer();

 protected:
  // 当插入到前面时候需要做什么
  virtual void onTimerInsertedAtFront() = 0;

  static void OnTimer(std::weak_ptr<void> weak_cond, Callback cb);

 private:
  RWMutexType m_mutex;
  std::set<Timer::ptr, Timer::Comparator> m_timers;
  bool m_tickled = false;
  uint64_t m_previous_time = 0;
};

}  // namespace ddg

#endif
