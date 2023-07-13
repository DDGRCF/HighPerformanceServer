#include <assert.h>
#include "ddg/log.h"
#include "ddg/utils.h"

auto g_logger = DDG_LOG_ROOT();

void test_assert() {
  DDG_LOG_INFO(g_logger) << ddg::BacktraceToString();
}

void test_string_util() {
  std::string test = "abcd11111abcd";
  std::string test_url = "//example.com:8042/over/there?name=ferret#nose";
  DDG_LOG_DEBUG(g_logger) << "StartWith Test "
                          << (ddg::StringUtil::StartWith(test, "abcd")
                                  ? "Success"
                                  : "Fail");
  DDG_LOG_DEBUG(g_logger) << "StartWith Test "
                          << (ddg::StringUtil::StartWith(test, "abcc")
                                  ? "Fail"
                                  : "Success");
  DDG_LOG_DEBUG(g_logger) << "EndWith Test "
                          << (ddg::StringUtil::EndWith(test, "abcd") ? "Success"
                                                                     : "Fail");
  DDG_LOG_DEBUG(g_logger) << "EndWith Test "
                          << (ddg::StringUtil::EndWith(test, "abcc")
                                  ? "Fail"
                                  : "Success");
  DDG_LOG_DEBUG(g_logger) << "ToUpper Test " << ddg::StringUtil::ToUpper(test);
  DDG_LOG_DEBUG(g_logger) << "ToLower Test " << ddg::StringUtil::ToLower(test);
  DDG_LOG_DEBUG(g_logger) << "Concat with [" << test << "] and [" << test
                          << "] " << ddg::StringUtil::Concat(",", test, test);
  DDG_LOG_DEBUG(g_logger) << "Concat with [" << test << "] and [" << test
                          << "] " << ddg::StringUtil::Concat(" | ", test, test);
  DDG_LOG_DEBUG(g_logger) << "Concat with [" << test << "] and [" << test
                          << "] " << ddg::StringUtil::Concat('|', test, test);
  test_url = ddg::StringUtil::UrlEncode(test_url);
  DDG_LOG_DEBUG(g_logger) << "UrlEncode Test " << test_url;
  DDG_LOG_DEBUG(g_logger) << "UrlDecode Test "
                          << ddg::StringUtil::UrlDecode(test_url);
}

int main() {
  test_assert();
  test_string_util();
  return 0;
}
