#ifndef DDG_UTILS_H_
#define DDG_UTILS_H_

#include <cxxabi.h>
#include <stdint.h>

namespace ddg {

uint64_t getThreadId();

uint64_t getFiberId();

template <typename T>
const char* TypeToName() {
  static const char* s_name =
      abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
  return s_name;
}

}  // namespace ddg

#endif
