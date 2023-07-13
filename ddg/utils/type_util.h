#ifndef DDG_UTILS_TYPE_UTIL_H__
#define DDG_UTILS_TYPE_UTIL_H__

#include <stdlib.h>
#include <string>

namespace ddg {

class TypeUtil {
 public:
  static int8_t ToChar(const std::string& str);

  static int64_t Atoi(const std::string& str);

  static double Atof(const std::string& str);

  static int8_t ToChar(const char* str);

  static int64_t Atoi(const char* str);

  static double Atof(const char* str);
};

}  // namespace ddg
#endif
