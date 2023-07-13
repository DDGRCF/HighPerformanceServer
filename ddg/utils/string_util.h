#ifndef DDG_UTILS_STRING_UTIL_H__
#define DDG_UTILS_STRING_UTIL_H__

#include <stdlib.h>
#include <string>

namespace ddg {

class StringUtil {
 private:
  static std::string Concat(const std::string& delim,
                            const std::initializer_list<std::string>& paths);

  static std::string Formatv(const char* fmt, va_list ap);

 public:
  static std::string Format(const char* fmt, ...);

  template <class... Args>
  static std::string Concat(const std::string& delim, Args&&... args) {
    return Concat(delim, {std::forward<Args>(args)...});
  }

  template <class... Args>
  static std::string Concat(const char delim, Args&&... args) {
    return Concat(std::string(1, delim), {std::forward<Args>(args)...});
  }

  // static std::string Split(const std::string& str, char delimit = ' ');

  static bool StartWith(const std::string& str, const std::string& prefix);

  static bool EndWith(const std::string& str, const std::string& suffix);

  static std::string UrlEncode(const std::string& str,
                               bool space_as_plus = true);

  static std::string UrlDecode(const std::string& str,
                               bool space_as_plus = true);

  static std::string Trim(const std::string& str,
                          const std::string& delimit = "\t\r\n");

  static std::string TrimLeft(const std::string& str,
                              const std::string& delimit = "\t\r\n");

  static std::string TrimRight(const std::string& str,
                               const std::string& delimit = "\t\r\n");

  static std::string ToUpper(const std::string& name);

  static std::string ToLower(const std::string& name);

  static const char* Time2Str(time_t ts, const char* format, char** buf,
                              int len);

  static std::string Time2Str(time_t ts,
                              const std::string& format = "%Y-%m-%d %H:%M:%S");

  static time_t Str2Time(const char* str,
                         const char* format = "%Y-%m-%d %H:%M:%S");

  static time_t Str2Time(const std::string& str,
                         const std::string& format = "%Y-%m-%d %H:%M:%S");
};

}  // namespace ddg
#endif
