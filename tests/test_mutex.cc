#include "ddg/log.h"
#include "ddg/mutex.h"
#include "pthread.h"

static int sum = 0;

static auto g_logger = DDG_LOG_ROOT();

static ddg::SpinLock spinlock;
static ddg::CASLock caslock;
static ddg::Mutex mutexlock;
static ddg::RWMutex rwlock;

void* run(void* arg) {
  for (int i = 0; i < 500; i++) {
    ddg::RWMutex::WriteLock lock(rwlock);
    // ddg::SpinLock::Lock lock(spinlock);
    // ddg::Mutex::Lock lock(mutexlock);
    sum++;
  }

  return static_cast<void*>(0);
}

void* rdrun(void* arg) {
  ddg::RWMutex::ReadLock lock(rwlock);
  for (int i = 0; i < 5; i++) {
    sleep(1);
    DDG_LOG_INFO(g_logger) << " read: " << sum;
  }

  return static_cast<void*>(0);
}

static const int kNum = 20;

int main() {
  int ret = 0;

  pthread_t tid[kNum + 2];
  for (int i = 0; i < kNum; i++) {
    ret = pthread_create(&tid[i], nullptr, run, nullptr);
    if (ret != 0) {
      DDG_LOG_ERROR(g_logger) << "pthread_create error";
      exit(1);
    }
  }

  for (int i = 0; i < 2; i++) {
    ret = pthread_create(&tid[i + kNum], nullptr, rdrun, nullptr);
    if (ret != 0) {
      DDG_LOG_ERROR(g_logger) << "pthread_create error";
      exit(1);
    }
  }

  for (int i = 0; i < kNum + 2; i++) {
    ret = pthread_join(tid[i], nullptr);
    if (ret != 0) {
      DDG_LOG_ERROR(g_logger) << "pthread_join error";
      throw std::logic_error("pthread_join error");
    }
  }

  DDG_LOG_INFO(g_logger) << "target: " << kNum * 500 << " | result: " << sum
                         << std::boolalpha
                         << " | passed: " << (kNum * 500 == sum);

  return 0;
}
