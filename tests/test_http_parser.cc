#include "ddg/http/http.h"
#include "ddg/http/http_parser.h"
#include "ddg/log.h"

const char test_request_data[] =
    "POST / HTTP/1.1\r\n"
    "host: www.sylar.top\r\n"
    "content-length: 10\r\n"
    "\r\n"
    "12345672222";

static ddg::Logger::ptr g_logger = DDG_LOG_ROOT();

void test_request() {
  ddg::http::HttpRequestParser parser;
  std::string tmp = test_request_data;

  size_t s = parser.execute(&tmp[0], tmp.size(), true);

  DDG_LOG_DEBUG(g_logger) << "execute ret = " << s
                          << " has_error = " << parser.getError()
                          << " is_finished = " << std::boolalpha
                          << parser.isFinished() << " total = " << tmp.size()
                          << " content_length = " << parser.getContentLength();

  tmp.resize(tmp.size() - s);

  DDG_LOG_INFO(g_logger) << "\n" << *parser.getData();
  DDG_LOG_INFO(g_logger) << "\n" << tmp;
}

int main() {
  test_request();
  return 0;
}
