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
    //
    WRITE_BUFFER_FULL = 4,
    READ_BUFFER_EMPTY = 5,
    /// 连接被对端关闭
    SEND_CLOSE_BY_PEER = 6,
    /// 发送请求产生Socket错误
    SEND_SOCKET_ERROR = 7,
    /// 超时
    TIMEOUT = 8,
    /// 创建Socket失败
    CREATE_SOCKET_ERROR = 9,
    /// 从连接池中取连接失败
    POOL_GET_CONNECTION = 10,
    /// 无效的连接
    POOL_INVALID_CONNECTION = 11,

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

  // 从req取得大部分信息，从uri指定地址信息 P77 18:00
  static HttpResult::ptr DoRequest(HttpRequest::ptr req, Uri::ptr uri,
                                   time_t timeout_ms);

 private:
  time_t m_createtime = 0;
  time_t m_request = 0;
};

// 为一个域名创建连接池
class HttpConnectionPool {
 public:
  using ptr = std::shared_ptr<HttpConnectionPool>;
  using MutexType = Mutex;

  HttpConnectionPool(const std::string& host, const std::string& vhost,
                     uint16_t port, bool is_https, size_t max_size,
                     time_t max_alive_time, size_t max_request);

  ~HttpConnectionPool();

  HttpConnection::ptr getConnection();

 public:
  static HttpConnectionPool::ptr Create(const std::string& uri,
                                        const std::string& vhost,
                                        size_t max_size, time_t max_alive_time,
                                        size_t max_request);

 private:
  static void ReleasePtr(HttpConnection* ptr, HttpConnectionPool* pool);

 public:
  HttpResult::ptr doGet(const std::string& url, time_t timeout_ms,
                        const std::map<std::string, std::string>& headers = {},
                        const std::string& body = "");

  HttpResult::ptr doGet(Uri::ptr uri, time_t timeout_ms,
                        const std::map<std::string, std::string>& headers = {},
                        const std::string& body = "");

  HttpResult::ptr doPost(const std::string& url, time_t timeout_ms,
                         const std::map<std::string, std::string>& headers = {},
                         const std::string& body = "");

  HttpResult::ptr doPost(Uri::ptr uri, time_t timeout_ms,
                         const std::map<std::string, std::string>& headers = {},
                         const std::string& body = "");

  HttpResult::ptr doRequest(
      HttpMethod method, const std::string& url, time_t timeout_ms,
      const std::map<std::string, std::string>& headers = {},
      const std::string& body = "");

  HttpResult::ptr doRequest(
      HttpMethod method, Uri::ptr uri, time_t timeout_ms,
      const std::map<std::string, std::string>& headers = {},
      const std::string& body = "");

  HttpResult::ptr doRequest(HttpRequest::ptr req, time_t timeout_ms);

 private:
  std::string m_host;
  // TODO: 一个连接支持不同的协议，不同协议的host不一样，因此有时候需要手动赋值
  std::string m_vhost;
  uint16_t m_port;
  uint32_t m_maxsize;
  time_t m_maxalivetime;
  uint32_t m_maxrequest;
  bool m_ishttps;

  MutexType m_mutex;
  std::list<HttpConnection*> m_conns;
  std::atomic<int32_t> m_total = {0};
};

}  // namespace http

}  // namespace ddg
#endif
