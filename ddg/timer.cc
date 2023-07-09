#include "ddg/timer.h"
#include "ddg/log.h"
#include "ddg/mutex.h"
#include "ddg/utils.h"

namespace ddg {

static Logger::ptr g_logger = DDG_LOG_ROOT();

// 这里需要改造，改造成观察者模式
Timer::Timer(uint64_t ms, Callback cb, bool recursing, TimerManager* manager)
    : m_ms(ms), m_cb(cb), m_recursing(recursing) {
  m_next = ddg::GetCurrentMilliSecond() + m_ms;
  m_manager = manager;
}

Timer::Timer(uint64_t next) : m_next(next) {}

bool Timer::cancel() {
  TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
  if (m_cb) {
    m_cb = nullptr;
    auto it = m_manager->m_timers.find(shared_from_this());
    m_manager->m_timers.erase(it);
    return true;
  }
  return false;
}

bool Timer::refresh() {
  TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
  if (!m_cb) {
    return false;
  }

  auto it = m_manager->m_timers.find(shared_from_this());
  if (it != m_manager->m_timers.end()) {
    return false;
  }

  m_manager->m_timers.erase(it);
  m_next = ddg::GetCurrentMilliSecond() + m_ms;
  m_manager->m_timers.insert(shared_from_this());
  return true;
}

bool Timer::reset(uint64_t ms, bool from_now) {
  uint64_t cur_time =
      ddg::GetCurrentMilliSecond();  // 当前函数的执行时间，都是从进入函数开始

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
  m_manager->addTimer(shared_from_this(), lock);
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
  m_previousTime = ddg::GetCurrentMilliSecond();
}

TimerManager::~TimerManager() {}

Timer::ptr TimerManager::addTimer(uint64_t ms, Callback cb, bool recursing) {
  Timer::ptr timer = std::make_shared<Timer>(ms, cb, recursing, this);
  RWMutexType::WriteLock lock(m_mutex);
  addTimer(timer, lock);
  return timer;
}

void TimerManager::addTimer(Timer::ptr val, RWMutexType::WriteLock& lock) {
  auto res = m_timers.insert(val);
  if (!res.second) {
    throw std::logic_error("TimerManage::addTimer insert m_timers fail");
  }

  auto it = res.first;

  bool at_front = (it == m_timers.begin()) && !m_tickled;
  if (at_front) {
    m_tickled = true;
  }

  lock.unlock();

  if (at_front) {
    onTimerInsertedAtFront();
  }
}

Timer::ptr TimerManager::addConditionTimer(uint64_t ms, Callback cb,
                                           std::weak_ptr<void> weak_cond,
                                           bool recurring) {
  return addTimer(ms, std::bind(&OnTimer, weak_cond, cb), recurring);
}

uint64_t TimerManager::getNextTimer() {
  uint64_t curr_ms = ddg::GetCurrentMilliSecond();

  RWMutexType::ReadLock lock(m_mutex);
  m_tickled = false;
  if (m_timers.empty()) {
    return ~0ull;
  }

  const Timer::ptr next = *m_timers.begin();

  if (curr_ms == next->m_next) {
    return 0;
  }

  return next->m_next - curr_ms;
}

void TimerManager::listExpiredCallback(std::vector<Callback>& cbs) {
  uint64_t curr_ms = ddg::GetCurrentMilliSecond();

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

  expired.insert(expired.begin(), m_timers.begin(), it);

  m_timers.erase(m_timers.begin(),
                 it);  // 平时不直接使用erase，是因为要满足remove_if的需求

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

bool TimerManager::detectClockRollover(uint64_t now_ms) {
  bool rollover = false;
  if (now_ms < m_previousTime &&
      now_ms < (m_previousTime - 60 * 60 * 1000)) {  // 大于一个小时以上
    rollover = true;
  }

  m_previousTime = now_ms;
  return rollover;
}

}  // namespace ddg
