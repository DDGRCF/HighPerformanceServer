#ifndef DDG_MUTEX_H_
#define DDG_MUTEX_H_

#include <memory>

#include <atomic>
#include "ddg/noncopyable.h"
#include "pthread.h"
#include "semaphore.h"
#include "stdint.h"

#define SCOPE_LOCK(lock, command) \
  {                               \
    lock;                         \
    command;                      \
  }

namespace ddg {

class Semphore : virtual public NonCopyable {
 public:
  Semphore(uint32_t count = 0);

  ~Semphore();

  void wait();

  void post();

 private:
  sem_t m_sem;
};

class CustomLock {
 public:
  CustomLock() = default;

  virtual ~CustomLock() = default;

  virtual void lock() = 0;

  virtual void unlock() = 0;
};

class CustomRWLock {
 public:
  CustomRWLock() = default;

  virtual ~CustomRWLock() = default;

  virtual void rdlock() = 0;

  virtual void wrlock() = 0;

  virtual void unlock() = 0;
};

template <class T>
class ScopedLockImpl : virtual public NonCopyable, CustomLock {
 public:
  explicit ScopedLockImpl(T& mutex) : m_mutex(mutex) { m_mutex.lock(); }

  explicit ScopedLockImpl(T* mutex) : m_mutex(*mutex) { m_mutex.lock(); }

  ~ScopedLockImpl() { unlock(); }

 protected:
  void lock() override { m_mutex.lock(); }

  void unlock() override { m_mutex.unlock(); }

  T& m_mutex;
};

template <class T>
class ReadScopedLockImpl {
 public:
  ReadScopedLockImpl(T& mutex) : m_mutex(mutex) { m_mutex.rdlock(); }

  ~ReadScopedLockImpl() { unlock(); }

 protected:
  void lock() { m_mutex.rdlock(); }

  void unlock() { m_mutex.unlock(); }

 private:
  T& m_mutex;
};

template <class T>
class WriteScopedLockImpl {
 public:
  WriteScopedLockImpl(T& mutex) : m_mutex(mutex) { m_mutex.wrlock(); }

  ~WriteScopedLockImpl() { unlock(); }

 protected:
  void lock() { m_mutex.wrlock(); }

  void unlock() { m_mutex.unlock(); }

 private:
  T& m_mutex;
};

class NullLock : public NonCopyable, CustomLock {
 public:
  using Lock = ScopedLockImpl<NullLock>;

  NullLock(){};
  ~NullLock(){};

  void lock() override{};
  void unlock() override{};
};

class Mutex : public NonCopyable, CustomLock {
 public:
  using Lock = ScopedLockImpl<Mutex>;

  Mutex();

  ~Mutex();

  void lock() override;

  void unlock() override;

 private:
  pthread_mutex_t m_mutex;
};

class CASLock : public NonCopyable, CustomLock {
 public:
  using Lock = ScopedLockImpl<CASLock>;

  CASLock();

  ~CASLock();

  void lock() override;

  void unlock() override;

 private:
  volatile std::atomic<bool> m_flag;
};

// 尝试过几次过后会把cpu给释放掉
class SpinLock : public NonCopyable, CustomLock {
 public:
  using Lock = ScopedLockImpl<SpinLock>;

  SpinLock();

  ~SpinLock();

  void lock() override;

  void unlock() override;

 private:
  pthread_spinlock_t m_mutex;
};

class RWMutex : public NonCopyable, CustomRWLock {
 public:
  using ReadLock = ReadScopedLockImpl<RWMutex>;
  using WriteLock = WriteScopedLockImpl<RWMutex>;

  RWMutex();
  ~RWMutex();

  void rdlock() override;
  void wrlock() override;
  void unlock() override;

 private:
  pthread_rwlock_t m_lock;
};

}  // namespace ddg

#endif
