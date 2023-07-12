#ifndef DDG_UTILS_H_
#define DDG_UTILS_H_

#include <cxxabi.h>
#include <stdint.h>
#include <string>
#include <vector>

#include "ddg/noncopyable.h"

namespace ddg {

pid_t GetThreadId();

pid_t GetFiberId();

template <typename T>
const char* TypeToName() {
  static const char* s_name =
      abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
  return s_name;
}

std::string BacktraceToString(int size = 64, int skip = 1,
                              const std::string& prefix = "");

time_t GetCurrentMicroSecond();

time_t GetCurrentMilliSecond();

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

}  // namespace ddg

#endif
