#ifndef DDG_UTILS_H_
#define DDG_UTILS_H_

#include <cxxabi.h>
#include <stdint.h>
#include <algorithm>
#include <numeric>
#include <string>
#include <vector>

#include "ddg/noncopyable.h"

namespace ddg {

class ScopedMalloc : public NonCopyable {
 public:
  explicit ScopedMalloc(size_t size) noexcept;
  ~ScopedMalloc();

  template <class T>
  T getPointer() const {
    return static_cast<T>(m_vptr);
  }

  void* getRawPointer() const { return m_vptr; }

 private:
  void* m_vptr;
};

uint64_t GetThreadId();

uint64_t GetFiberId();

template <typename T>
const char* TypeToName() {
  static const char* s_name =
      abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
  return s_name;
}

class StringUtil {
 private:
  static std::string Concat(const std::initializer_list<std::string>& paths);

  static std::string Formatv(const char* fmt, va_list ap);

 public:
  static std::string Format(const char* fmt, ...);

  template <class... Args>
  static std::string Concat(Args&&... args) {
    int dummy[] = {(std::is_convertible<Args, std::string>::value, 0)...};
    reinterpret_cast<void>(dummy);
    return Concat({args...});
  }

  // static std::string Split(const std::string& str, char delimit = ' ');
  //
  // static std::string StartWith(const std::string& str,
  // const std::string& prefix);
  //
  // static std::string EndWith(const std::string& str, const std::string& suffix);

  // static std::string UrlEncode(const std::string& str,
  // bool space_as_plus = true);
  //
  // static std::string UrlDecode(const std::string& str,
  // bool space_as_plus = true);

  static std::string Trim(const std::string& str,
                          const std::string& delimit = "\t\r\n");

  static std::string TrimLeft(const std::string& str,
                              const std::string& delimit = "\t\r\n");

  static std::string TrimRight(const std::string& str,
                               const std::string& delimit = "\t\r\n");
};

std::string BacktraceToString(int size = 64, int skip = 1,
                              const std::string& prefix = "");

uint64_t GetCurrentMicroSecond();

uint64_t GetCurrentMilliSecond();

const char* Time2Str(time_t ts, const char* format, const char** buf, int len);

std::string Time2Str(time_t ts, const std::string& format);

time_t Str2Time(const char* str, const char* format);

time_t Str2Time(const std::string& str, const std::string& format);

std::string ToUpper(const std::string& name);

std::string ToLower(const std::string& name);

}  // namespace ddg

#endif
