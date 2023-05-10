#include "ddg/mutex.h"
#include "log.h"

namespace ddg {

auto g_logger = DDG_LOG_ROOT();

// Semphore
Semphore::Semphore(uint32_t count) {
  if (sem_init(&m_sem, 0, count)) {
    throw std::logic_error("sem_init error");
  }
}

Semphore::~Semphore() {
  sem_destroy(&m_sem);
}

void Semphore::wait() {
  if (sem_wait(&m_sem)) {
    std::logic_error("sem_wait error");
  }
}

void Semphore::post() {
  if (sem_post(&m_sem)) {
    std::logic_error("sem_post error");
  }
}

Mutex::Mutex() {
  if (pthread_mutex_init(&m_mutex, nullptr)) {
    std::logic_error("pthread_mutex_init error");
  }
}

Mutex::~Mutex() {
  pthread_mutex_destroy(&m_mutex);
}

void Mutex::lock() {
  if (pthread_mutex_lock(&m_mutex)) {
    std::logic_error("pthread_mutex_lock error");
  }
}

void Mutex::unlock() {
  if (pthread_mutex_unlock(&m_mutex)) {
    std::logic_error("pthread_mutex_unlock error");
  }
}

CASLock::CASLock() : m_flag(false) {}

CASLock::~CASLock() {}

void CASLock::lock() {
  bool expect = false;
  while (!m_flag.compare_exchange_weak(expect, true)) {
    expect = false;
  }
}

void CASLock::unlock() {
  m_flag.store(false);
}

SpinLock::SpinLock() {
  if (pthread_spin_init(&m_mutex, PTHREAD_PROCESS_PRIVATE)) {
    std::logic_error("pthread_spin_init error");
  }
}

SpinLock::~SpinLock() {}

void SpinLock::lock() {
  if (pthread_spin_lock(&m_mutex)) {
    std::logic_error("pthread_spin_lock error");
  }
}

void SpinLock::unlock() {
  if (pthread_spin_unlock(&m_mutex)) {
    std::logic_error("pthread_spin_unlock error");
  }
}

RWMutex::RWMutex() {
  if (pthread_rwlock_init(&m_lock, nullptr)) {
    std::logic_error("pthread_rwlock_init error");
  }
}

void RWMutex::rdlock() {
  if (pthread_rwlock_rdlock(&m_lock)) {
    std::logic_error("pthread_rwlock_rdlock error");
  }
}

void RWMutex::wrlock() {
  if (pthread_rwlock_wrlock(&m_lock)) {
    std::logic_error("pthread_rwlock_wrlock error");
  }
}

void RWMutex::unlock() {
  if (pthread_rwlock_unlock(&m_lock)) {
    std::logic_error("pthread_rwlock_unlock error");
  }
}

RWMutex::~RWMutex() {
  pthread_rwlock_destroy(&m_lock);
}

}  // namespace ddg
   //
