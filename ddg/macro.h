#ifndef DDG_LOG_MACRO_H_
#define DDG_LOG_MACRO_H_

#include <assert.h>
#include <string.h>
#include "ddg/log.h"
#include "ddg/utils.h"

// 使用!!()，将表达式转化为bool值
#if defined __GNUC__ || defined __llvm__
#define DDG_LIKELY(x) __builtin_expect(!!(x), 1)
#define DDG_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define DDG_LIKELY(x) (x)
#define DDG_UNLIKELY(x) (x)
#endif

#define DDG_ASSERT(x)                                            \
  if (DDG_UNLIKELY(!(x))) {                                      \
    DDG_LOG_ERROR(DDG_LOG_NAME("system"))                        \
        << "ASSERTION: " #x << "\nbacktrace:\n"                  \
        << ddg::BacktraceToString(100, 2, std::string(10, ' ')); \
    assert(x);                                                   \
  }

#define DDG_ASSERT_MSG(x, msg)                                   \
  if (DDG_UNLIKELY(!(x))) {                                      \
    DDG_LOG_ERROR(DDG_LOG_NAME("system"))                        \
        << "ASSERTION: " #x << "\n"                              \
        << msg << "\nbacktrace:\n"                               \
        << ddg::BacktraceToString(100, 2, std::string(10, ' ')); \
    assert(x);                                                   \
  }
#endif
