#ifndef DDG_HTTP_HTTP_CONNECTION_H_
#define DDG_HTTP_HTTP_CONNECTION_H_

#include <memory>
#include <string>

#include "ddg/http/http.h"
#include "ddg/log.h"
#include "ddg/stream/socket_stream.h"
#include "ddg/uri.h"

namespace ddg {

static ddg::Logger::ptr g_logger = DDG_LOG_ROOT();

namespace http {

struct HttpResult {
  using ptr = std::shared_ptr<HttpResult>;

  enum class Error {
    /// 正常
    OK = 0,
    /// 非法URL
    INVALID_URL = 1,
    /// 无法解析HOST
    INVALID_HOST = 2,
    /// 连接失败
    CONNECT_FAIL = 3,
    /// 连接被对端关闭
    SEND_CLOSE_BY_PEER = 4,
    /// 发送请求产生Socket错误
    SEND_SOCKET_ERROR = 5,
    /// 超时
    TIMEOUT = 6,
    /// 创建Socket失败
    CREATE_SOCKET_ERROR = 7,
    /// 从连接池中取连接失败
    POOL_GET_CONNECTION = 8,
    /// 无效的连接
    POOL_INVALID_CONNECTION = 9,

  };

  HttpResult(int _result, HttpResponse::ptr _response,
             const std::string& _error);
  ~HttpResult();

  /// 错误码
  int result;
  /// HTTP响应结构体
  HttpResponse::ptr response;
  /// 错误描述
  std::string error;

  std::string toString() const;

  friend std::ostream& operator<<(std::ostream& os, const HttpResult& result);
};

class HttpConnectionPool;

class HttpConnection : public SocketStream {
 public:
  friend class HttpConnectionPool;

  using ptr = std::shared_ptr<HttpConnection>;

  HttpConnection(Socket::ptr sock, bool owner = true);

  ~HttpConnection();

  int sendRequest(HttpRequest::ptr req);

  HttpResponse::ptr recvResponse();

 public:
  static HttpResult::ptr DoGet(
      const std::string& url, time_t timeout_ms,
      const std::map<std::string, std::string>& headers = {},
      const std::string& body = "");

  static HttpResult::ptr DoGet(
      Uri::ptr uri, time_t timeout_ms,
      const std::map<std::string, std::string>& headers = {},
      const std::string& body = "");

  static HttpResult::ptr DoPost(
      const std::string& url, time_t timeout_ms,
      const std ::map<std::string, std::string>& headers = {},
      const std::string& body = "");

  static HttpResult::ptr DoPost(
      Uri::ptr uri, time_t timeout_ms,
      const std ::map<std::string, std::string>& headers = {},
      const std::string& body = "");

  static HttpResult::ptr DoRequest(
      HttpMethod method, const std::string& url, time_t timeout_ms,
      const std::map<std::string, std::string>& headers = {},
      const std::string& body = "");

  static HttpResult::ptr DoRequest(
      HttpMethod method, Uri::ptr uri, time_t timeout_ms,
      const std::map<std::string, std::string>& headers = {},
      const std::string& body = "");

  static HttpResult::ptr DoRequest(HttpRequest::ptr req, Uri::ptr uri,
                                   time_t timeout_ms);

 private:
  time_t m_createtime = 0;
  time_t m_request = 0;
};

}  // namespace http

}  // namespace ddg
#endif
