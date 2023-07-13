#ifndef DDG_HTTP_HTTP_PARSER_H_
#define DDG_HTTP_HTTP_PARSER_H_

#include "http.h"
#include "http11_parser.h"
#include "httpclient_parser.h"

#include <memory>

namespace ddg {
namespace http {

class HttpRequestParser {
 public:
  using ptr = std::shared_ptr<HttpRequestParser>;

  HttpRequestParser();

  size_t execute(char* data, size_t len, bool withbody = false);

  bool isFinished();

  HttpRequest::ptr getData() const;

  int getError();

  void setError(int v);

  uint64_t getContentLength();

  const http_parser& getParser() const;

 public:
  static uint64_t GetHttpRequestBufferSize();

  static uint64_t GetHttpRequestMaxBodySize();

 private:
  http_parser m_parser;

  HttpRequest::ptr m_data;

  int m_error;
};

class HttpResponseParser {
 public:
  using ptr = std::shared_ptr<HttpResponseParser>;

  HttpResponseParser();

  size_t execute(char* data, size_t len, bool chunck, bool withbody = false);

  bool isFinished();

  HttpResponse::ptr getData() const { return m_data; }

  int getError();

  void setError(int v) { m_error = v; }

  uint64_t getContentLength();

  const httpclient_parser& getParser() const { return m_parser; }

 public:
  static uint64_t GetHttpResponseBufferSize();

  static uint64_t GetHttpResponseMaxBodySize();

 private:
  httpclient_parser m_parser;

  HttpResponse::ptr m_data;

  int m_error;
};

}  // namespace http

}  // namespace ddg

#endif
