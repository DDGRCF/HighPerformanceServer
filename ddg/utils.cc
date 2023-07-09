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

const char* Time2Str(time_t ts, const char* format, char** buf, int len) {
  struct tm tm;
  localtime_r(&ts, &tm);
  strftime(*buf, len, format, &tm);
  return *buf;
}

std::string Time2Str(time_t ts, const std::string& format) {
  struct tm tm;
  localtime_r(&ts, &tm);
  char buf[64];
  strftime(buf, sizeof(buf), format.c_str(), &tm);
  return buf;
}

time_t Str2Time(const char* str, const char* format) {
  struct tm t;
  bzero(&t, sizeof(t));
  if (!strptime(str, format, &t)) {
    return 0;
  }
  return mktime(&t);
}

time_t Str2Time(const std::string& str, const std::string& format) {
  struct tm t;
  bzero(&t, sizeof(t));
  if (!strptime(str.c_str(), format.c_str(), &t)) {
    return 0;
  }
  return mktime(&t);
}

std::string ToUpper(const std::string& name) {
  std::string ret = name;
  std::transform(ret.begin(), ret.end(), ret.begin(), ::toupper);
  return ret;
}

std::string ToLower(const std::string& name) {
  std::string ret = name;
  std::transform(ret.begin(), ret.end(), ret.begin(), ::tolower);
  return ret;
}

std::string StringUtil::Concat(
    const std::initializer_list<std::string>& paths) {
  return std::accumulate(paths.begin(), paths.end(), std::string(""),
                         [](const std::string& x, const std::string& y) {
                           return x.empty() ? y : x + y;
                         });
}

std::string StringUtil::Format(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  auto v = Formatv(fmt, ap);
  va_end(ap);
  return v;
}

std::string StringUtil::Formatv(const char* fmt, va_list ap) {
  char* buf = nullptr;
  auto len = vasprintf(&buf, fmt, ap);
  if (len == -1) {
    return "";
  }
  std::string ret(buf, len);
  free(buf);
  return ret;
}

std::string StringUtil::Trim(const std::string& str,
                             const std::string& delimit) {
  auto begin = str.find_first_not_of(delimit);
  if (begin == std::string::npos) {
    return "";
  }
  auto end = str.find_last_not_of(delimit);
  return str.substr(begin, end - begin + 1);
}

std::string StringUtil::TrimLeft(const std::string& str,
                                 const std::string& delimit) {
  auto begin = str.find_first_not_of(delimit);
  if (begin == std::string::npos) {
    return "";
  }
  return str.substr(begin);
}

std::string StringUtil::TrimRight(const std::string& str,
                                  const std::string& delimit) {
  auto end = str.find_last_not_of(delimit);
  if (end == std::string::npos) {
    return "";
  }
  return str.substr(0, end);
}
}  // namespace ddg
