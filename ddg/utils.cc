#include "utils.h"

#include <execinfo.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <thread>

#include "ddg/fiber.h"
#include "ddg/log.h"

namespace ddg {

uint64_t GetThreadId() {
  return syscall(SYS_gettid);
}

uint64_t GetFiberId() {
  return ddg::Fiber::GetFiberId();
}

void BackTrace(std::vector<std::string>& bt, int size, int skip) {
  ScopedMalloc sm(sizeof(void*) * size);
  void** array = sm.getPointer<void**>();

  size_t nptrs = ::backtrace(array, size);
  char** strings = backtrace_symbols(array, size);

  if (strings == nullptr) {
    std::system_error();
  }

  bt.resize(nptrs - skip + 1);
  for (size_t j = skip; j < nptrs; j++) {
    bt[j - skip] = strings[j];
  }

  free(strings);
}

std::string BacktraceToString(int size, int skip, const std::string& prefix) {
  std::vector<std::string> bt;
  BackTrace(bt, size, skip);
  std::stringstream ss;
  for (size_t i = 0; i < bt.size(); i++) {
    ss << prefix << bt[i] << "\n";
  }
  return ss.str();
}

ScopedMalloc::ScopedMalloc(size_t size) noexcept {
  m_vptr = malloc(size);
}

ScopedMalloc::~ScopedMalloc() {
  free(m_vptr);
}

uint64_t GetCurrentMicroSecond() {
  struct timeval tv;
  int ret = gettimeofday(&tv, nullptr);
  if (!ret) {
    throw std::system_error();
  }

  return tv.tv_sec * 1000 * 1000ul + tv.tv_usec;
}

uint64_t GetCurrentMilliSecond() {
  struct timeval tv;
  int ret = gettimeofday(&tv, nullptr);
  if (ret) {
    throw std::system_error();
  }
  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

}  // namespace ddg
