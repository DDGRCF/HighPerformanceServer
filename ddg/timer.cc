#include "ddg/timer.h"
#include <asm-generic/errno.h>

#include <system_error>

#include "ddg/log.h"
#include "ddg/mutex.h"
#include "ddg/utils.h"

namespace ddg {

static Logger::ptr g_logger = DDG_LOG_ROOT();

Timer::Timer(time_t ms, Callback cb, bool recursing, TimerManager* manager)
    : m_cb(cb), m_recursing(recursing), m_ms(ms), m_manager(manager) {
  m_next = ddg::GetCurrentMilliSecond() + m_ms;
}

Timer::Timer(time_t next) : m_next(next) {}

// 将定时器自身取消
bool Timer::cancel() {
  TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
  // 如果定时器自身有m_cb就移除
  if (m_cb) {
    m_cb = nullptr;
    auto it = m_manager->m_timers.find(shared_from_this());
    m_manager->m_timers.erase(it);
    return true;
  }
  return false;
}

// 更新定时器，先移除再插入，更新为重线程开始计算的时间
bool Timer::refresh() {
  TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
  if (!m_cb) {
    return false;
  }

  auto it = m_manager->m_timers.find(shared_from_this());
  if (it == m_manager->m_timers.end()) {
    return false;
  }

  m_manager->m_timers.erase(it);
  m_next = ddg::GetCurrentMilliSecond() + m_ms;
  m_manager->m_timers.insert(shared_from_this());
  return true;
}

// 重置定时时间，from_now代表从现在开始设置定时时间
bool Timer::reset(time_t ms, bool from_now) {
  // 当前函数的执行时间，都是从进入函数开始
  time_t cur_time = ddg::GetCurrentMilliSecond();

  if (ms == m_ms && !from_now) {
    return true;
  }

  TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);

  if (!m_cb) {
    return false;
  }

  auto it = m_manager->m_timers.find(shared_from_this());
  if (it == m_manager->m_timers.end()) {
    return false;
  }

  m_manager->m_timers.erase(it);

  if (from_now) {
    m_next = cur_time + ms;
  } else {
    m_next = m_next - m_ms + ms;
  }

  m_ms = ms;
  lock.unlock();

  m_manager->addTimer(shared_from_this());
  return true;
}

bool Timer::Comparator::operator()(const Timer::ptr& lhs,
                                   const Timer::ptr& rhs) const {
  if (!lhs && !rhs) {
    return false;
  }

  if (!lhs) {
    return true;
  }

  if (!rhs) {
    return false;
  }

  if (lhs->m_next < rhs->m_next) {
    return true;
  }

  if (lhs->m_next == rhs->m_next) {
    return lhs.get() < rhs.get();
  }

  return lhs->m_next < rhs->m_next;
};

TimerManager::TimerManager() {
  m_previous_time = ddg::GetCurrentMilliSecond();
}

TimerManager::~TimerManager() {}

Timer::ptr TimerManager::addTimer(time_t ms, Callback cb, bool recursing) {
  Timer::ptr timer(new Timer(ms, cb, recursing, this));
  addTimer(timer);
  return timer;
}

bool TimerManager::addTimerNoLock(Timer::ptr val) {
  auto res = m_timers.insert(val);
  if (!res.second) {
    throw std::logic_error("TimerManage::addTimer insert m_timers fail");
  }

  auto it = res.first;

  bool at_front = (it == m_timers.begin()) && !m_tickled;
  if (at_front) {
    m_tickled = true;
  }
  return at_front;
}

void TimerManager::addTimer(Timer::ptr val) {
  bool at_front = false;
  {
    RWMutexType::WriteLock lock(m_mutex);
    at_front = addTimerNoLock(val);
  }

  if (at_front) {
    onTimerInsertedAtFront();
  }
}

Timer::ptr TimerManager::addConditionTimer(time_t ms, Callback cb,
                                           std::weak_ptr<void> weak_cond,
                                           bool recursing) {
  return addTimer(ms, std::bind(&OnTimer, weak_cond, cb), recursing);
}

// 如果m_timers为空那么就返回-1，如果不为空就返回第一个定时器的下个时间
time_t TimerManager::getNextTimer() {
  time_t curr_ms = ddg::GetCurrentMilliSecond();

  RWMutexType::ReadLock lock(m_mutex);
  m_tickled = false;
  if (m_timers.empty()) {
    return -1;
  }

  const Timer::ptr next = *m_timers.begin();

  if (curr_ms == next->m_next) {
    return 0;
  }

  return next->m_next - curr_ms;
}

void TimerManager::listExpiredCallback(std::vector<Callback>& cbs) {
  time_t curr_ms = ddg::GetCurrentMilliSecond();

  std::vector<Timer::ptr> expired;
  {
    RWMutexType::ReadLock lock(m_mutex);
    if (m_timers.empty()) {
      return;
    }
  }

  RWMutexType::WriteLock lock(m_mutex);
  if (m_timers.empty()) {
    return;
  }

  bool rollover = detectClockRollover(curr_ms);
  if (!rollover && (*m_timers.begin())->m_next > curr_ms) {
    return;
  }

  Timer::ptr curr_timer(new Timer(curr_ms));

  auto it = rollover ? m_timers.end() : m_timers.upper_bound(curr_timer);

  // 插入过期时间
  expired.insert(expired.begin(), m_timers.begin(), it);

  // 平时不直接使用erase，是因为要满足remove_if的需求
  m_timers.erase(m_timers.begin(), it);

  // 将cbs扩展为cbs大小
  cbs.reserve(expired.size());

  for (auto& timer : expired) {  // 如果需要循环的话就重新设置时间
    cbs.push_back(timer->m_cb);
    if (timer->m_recursing) {
      timer->m_next = curr_ms + timer->m_ms;
      m_timers.insert(timer);
    } else {
      timer->m_cb = nullptr;
    }
  }
}

bool TimerManager::hasTimer() {
  RWMutexType::ReadLock lock(m_mutex);
  return m_timers.empty();
}

// 使用weak_cond没有过期才执行
void TimerManager::OnTimer(std::weak_ptr<void> weak_cond, Callback cb) {
  std::shared_ptr<void> tmp = weak_cond.lock();
  if (tmp) {
    cb();
  }
}

// 好像没有作用
bool TimerManager::detectClockRollover(time_t now_ms) {
  bool rollover = false;
  if (now_ms < m_previous_time) {
    rollover = true;
  }

  // 时间大于一个小时以上，就报错
  if (now_ms < m_previous_time - 60 * 60 * 1000) {
    throw std::logic_error("error system time");
  }

  m_previous_time = now_ms;
  return rollover;
}

}  // namespace ddg
