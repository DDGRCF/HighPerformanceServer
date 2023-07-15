#ifndef DDG_HTTP_HTTP_H_
#define DDG_HTTP_HTTP_H_

#include <map>
#include <memory>
#include <string>

#include <boost/lexical_cast.hpp>

#include "ddg/utils.h"

namespace ddg {
namespace http {

#define HTTP_METHOD_MAP(XX)        \
  XX(0, DELETE, DELETE)            \
  XX(1, GET, GET)                  \
  XX(2, HEAD, HEAD)                \
  XX(3, POST, POST)                \
  XX(4, PUT, PUT)                  \
  /* pathological */               \
  XX(5, CONNECT, CONNECT)          \
  XX(6, OPTIONS, OPTIONS)          \
  XX(7, TRACE, TRACE)              \
  /* WebDAV */                     \
  XX(8, COPY, COPY)                \
  XX(9, LOCK, LOCK)                \
  XX(10, MKCOL, MKCOL)             \
  XX(11, MOVE, MOVE)               \
  XX(12, PROPFIND, PROPFIND)       \
  XX(13, PROPPATCH, PROPPATCH)     \
  XX(14, SEARCH, SEARCH)           \
  XX(15, UNLOCK, UNLOCK)           \
  XX(16, BIND, BIND)               \
  XX(17, REBIND, REBIND)           \
  XX(18, UNBIND, UNBIND)           \
  XX(19, ACL, ACL)                 \
  /* subversion */                 \
  XX(20, REPORT, REPORT)           \
  XX(21, MKACTIVITY, MKACTIVITY)   \
  XX(22, CHECKOUT, CHECKOUT)       \
  XX(23, MERGE, MERGE)             \
  /* upnp */                       \
  XX(24, MSEARCH, M - SEARCH)      \
  XX(25, NOTIFY, NOTIFY)           \
  XX(26, SUBSCRIBE, SUBSCRIBE)     \
  XX(27, UNSUBSCRIBE, UNSUBSCRIBE) \
  /* RFC-5789 */                   \
  XX(28, PATCH, PATCH)             \
  XX(29, PURGE, PURGE)             \
  /* CalDAV */                     \
  XX(30, MKCALENDAR, MKCALENDAR)   \
  /* RFC-2068, section 19.6.1.2 */ \
  XX(31, LINK, LINK)               \
  XX(32, UNLINK, UNLINK)           \
  /* icecast */                    \
  XX(33, SOURCE, SOURCE)

#define HTTP_STATUS_MAP(XX)                                                 \
  XX(100, CONTINUE, Continue)                                               \
  XX(101, SWITCHING_PROTOCOLS, Switching Protocols)                         \
  XX(102, PROCESSING, Processing)                                           \
  XX(200, OK, OK)                                                           \
  XX(201, CREATED, Created)                                                 \
  XX(202, ACCEPTED, Accepted)                                               \
  XX(203, NON_AUTHORITATIVE_INFORMATION, Non - Authoritative Information)   \
  XX(204, NO_CONTENT, No Content)                                           \
  XX(205, RESET_CONTENT, Reset Content)                                     \
  XX(206, PARTIAL_CONTENT, Partial Content)                                 \
  XX(207, MULTI_STATUS, Multi - Status)                                     \
  XX(208, ALREADY_REPORTED, Already Reported)                               \
  XX(226, IM_USED, IM Used)                                                 \
  XX(300, MULTIPLE_CHOICES, Multiple Choices)                               \
  XX(301, MOVED_PERMANENTLY, Moved Permanently)                             \
  XX(302, FOUND, Found)                                                     \
  XX(303, SEE_OTHER, See Other)                                             \
  XX(304, NOT_MODIFIED, Not Modified)                                       \
  XX(305, USE_PROXY, Use Proxy)                                             \
  XX(307, TEMPORARY_REDIRECT, Temporary Redirect)                           \
  XX(308, PERMANENT_REDIRECT, Permanent Redirect)                           \
  XX(400, BAD_REQUEST, Bad Request)                                         \
  XX(401, UNAUTHORIZED, Unauthorized)                                       \
  XX(402, PAYMENT_REQUIRED, Payment Required)                               \
  XX(403, FORBIDDEN, Forbidden)                                             \
  XX(404, NOT_FOUND, Not Found)                                             \
  XX(405, METHOD_NOT_ALLOWED, Method Not Allowed)                           \
  XX(406, NOT_ACCEPTABLE, Not Acceptable)                                   \
  XX(407, PROXY_AUTHENTICATION_REQUIRED, Proxy Authentication Required)     \
  XX(408, REQUEST_TIMEOUT, Request Timeout)                                 \
  XX(409, CONFLICT, Conflict)                                               \
  XX(410, GONE, Gone)                                                       \
  XX(411, LENGTH_REQUIRED, Length Required)                                 \
  XX(412, PRECONDITION_FAILED, Precondition Failed)                         \
  XX(413, PAYLOAD_TOO_LARGE, Payload Too Large)                             \
  XX(414, URI_TOO_LONG, URI Too Long)                                       \
  XX(415, UNSUPPORTED_MEDIA_TYPE, Unsupported Media Type)                   \
  XX(416, RANGE_NOT_SATISFIABLE, Range Not Satisfiable)                     \
  XX(417, EXPECTATION_FAILED, Expectation Failed)                           \
  XX(421, MISDIRECTED_REQUEST, Misdirected Request)                         \
  XX(422, UNPROCESSABLE_ENTITY, Unprocessable Entity)                       \
  XX(423, LOCKED, Locked)                                                   \
  XX(424, FAILED_DEPENDENCY, Failed Dependency)                             \
  XX(426, UPGRADE_REQUIRED, Upgrade Required)                               \
  XX(428, PRECONDITION_REQUIRED, Precondition Required)                     \
  XX(429, TOO_MANY_REQUESTS, Too Many Requests)                             \
  XX(431, REQUEST_HEADER_FIELDS_TOO_LARGE, Request Header Fields Too Large) \
  XX(451, UNAVAILABLE_FOR_LEGAL_REASONS, Unavailable For Legal Reasons)     \
  XX(500, INTERNAL_SERVER_ERROR, Internal Server Error)                     \
  XX(501, NOT_IMPLEMENTED, Not Implemented)                                 \
  XX(502, BAD_GATEWAY, Bad Gateway)                                         \
  XX(503, SERVICE_UNAVAILABLE, Service Unavailable)                         \
  XX(504, GATEWAY_TIMEOUT, Gateway Timeout)                                 \
  XX(505, HTTP_VERSION_NOT_SUPPORTED, HTTP Version Not Supported)           \
  XX(506, VARIANT_ALSO_NEGOTIATES, Variant Also Negotiates)                 \
  XX(507, INSUFFICIENT_STORAGE, Insufficient Storage)                       \
  XX(508, LOOP_DETECTED, Loop Detected)                                     \
  XX(510, NOT_EXTENDED, Not Extended)                                       \
  XX(511, NETWORK_AUTHENTICATION_REQUIRED, Network Authentication Required)

enum class HttpMethod {
#define XX(num, name, string) name = num,
  HTTP_METHOD_MAP(XX)
#undef XX
      INVALID_METHOD
};

enum class HttpStatus {
#define XX(code, name, desc) name = code,
  HTTP_STATUS_MAP(XX)
#undef XX
};

HttpMethod StringToHttpMethod(const std::string& m);

HttpMethod CharsToHttpMethod(const char* m);

const char* HttpMethodToString(const HttpMethod& m);

const char* HttpStatusToString(const HttpStatus& s);

struct CaseInsensitiveLess {
  /**
     * @brief 忽略大小写比较字符串
     */
  bool operator()(const std::string& lhs, const std::string& rhs) const;
};

template <class MapType, class T>
bool checkGetAs(const MapType& m, const std::string& key, T& val,
                const T& def = T()) {
  auto it = m.find(key);
  if (it != m.end()) {
    val = def;
    return false;
  }

  try {
    val = boost::lexical_cast<T>(it->second);
    return true;
  } catch (...) {
    val = def;
  }
  return false;
}

template <class MapType, class T>
T getAs(const MapType& m, const std::string& key, const T& def = T()) {
  auto it = m.find(key);
  if (it == m.end()) {
    return def;
  }
  try {
    return boost::lexical_cast<T>(it->second);
  } catch (...) {
    return def;
  }
}

template <class MapType, class T>
void setAs(MapType& m, const std::string& key, const T& value) {
  std::string lowercasekey = StringUtil::ToLower(key);
  std::string lowercasevalue = StringUtil::ToLower(std::to_string(value));
  m[lowercasekey] = lowercasevalue;
}

template <class MapType>
void setAs(MapType& m, const std::string& key, const std::string& value) {
  auto lowercasekey = StringUtil::ToLower(key);
  auto lowercasevalue = StringUtil::ToLower(value);
  m[lowercasekey] = lowercasevalue;
}

class HttpResponse;

class HttpRequest {
 public:
  using ptr = std::shared_ptr<HttpRequest>;
  using MapType = std::map<std::string, std::string, CaseInsensitiveLess>;

  HttpRequest(uint8_t version = 0x11,
              bool close = true);  // TODO: 版本可以改变

  std::shared_ptr<HttpResponse> createResponse();

  std::ostream& dump(std::ostream& os) const;

  friend std::ostream& operator<<(std::ostream& os, const HttpRequest& req);

  std::string toString() const;

 public:
  HttpMethod getMethod() const;

  uint8_t getVersion() const;

  const std::string& getPath() const;

  const std::string& getQuery() const;

  const std::string& getBody() const;

  const MapType& getHeaders() const;

  const MapType& getParams() const;

  const MapType& getCookies() const;

 public:
  void setMethod(HttpMethod v);

  void setVersion(uint8_t v);

  void setPath(const std::string& v);

  void setQuery(const std::string& v);

  void setFragment(const std::string& v);

  void setBody(const std::string& v);

  void setHeaders(const MapType& v);

  void setParams(const MapType& v);

  void setCookies(const MapType& v);

  void setClose(bool v);

 public:
  std::string getHeader(const std::string& key, const std::string& def = "");

  std::string getParam(const std::string& key, const std::string& def = "");

  std::string getCookie(const std::string& key, const std::string& def = "");

  void setHeader(const std::string& key, const std::string& val);

  void setParam(const std::string& key, const std::string& val);

  void setCookie(const std::string& key, const std::string& val);

  void delHeader(const std::string& key);

  void delParam(const std::string& key);

  void delCookie(const std::string& key);

  bool hasHeader(const std::string& key, std::string* val = nullptr) const;

  bool hasParam(const std::string& key, std::string* val = nullptr) const;

  bool hasCookie(const std::string& key, std::string* val = nullptr) const;

  bool isClose() const;

  bool isWebsocket() const;

 public:
  template <class T>
  bool checkGetHeaderAs(const std::string& key, T& val,
                        const T& def = T()) const {
    return checkGetAs(m_headers, key, val, def);
  }

  template <class T>
  T getHeaderAs(const std::string& key, const T& def = T()) const {
    return getAs(m_headers, key, def);
  }

  template <class T>
  void setHeaderAs(const std::string& key, const T& value) {
    setAs(m_headers, key, value);
  }

  template <class T>
  bool checkGetParamAs(const std::string& key, T& val,
                       const T& def = T()) const {
    return checkGetAs(m_params, key, val, def);
  }

  template <class T>
  T getParamAs(const std::string& key, const T& def = T()) const {
    return getAs(m_params, key, def);
  }

  template <class T>
  void setParamAs(const std::string& key, const T& value) {
    setAs(m_params, key, value);
  }

  template <class T>
  bool checkGetCookieAs(const std::string& key, T& val,
                        const T& def = T()) const {
    return checkGetAs(m_cookies, key, val, def);
  }

  template <class T>
  T getCookieAs(const std::string& key, const T& def = T()) const {
    return getAs(m_cookies, key, def);
  }

  template <class T>
  void setCookieAs(const std::string& key, const T& value) {
    setAs(m_cookies, key, value);
  }

 public:
  void init();

  void initParam();

  void initQueryParam();

  void initBodyParam();

  void initCookies();

 private:
  HttpMethod m_method;

  uint8_t m_version;

  bool m_close;

  bool m_websocket;  // web socket

  bool m_ischunk;

  uint8_t m_parser_paramflag;

  std::string m_path;

  std::string m_query;

  std::string m_fragment;

  std::string m_body;

  MapType m_headers;

  MapType m_params;

  MapType m_cookies;
};

class HttpResponse {
 public:
  using ptr = std::shared_ptr<HttpResponse>;
  using MapType = std::map<std::string, std::string, CaseInsensitiveLess>;

  HttpResponse(uint8_t version = 0x11, bool close = true, bool chunk = false);

  friend std::ostream& operator<<(std::ostream& os, const HttpResponse& resp);

  std::ostream& dump(std::ostream& os) const;

  std::string toString() const;

 public:
  HttpStatus getStatus() const;

  uint8_t getVersion() const;

  const std::string& getBody() const;

  const std::string& getReason() const;

  const MapType& getHeaders() const;

  void setStatus(HttpStatus v);

  void setVersion(uint8_t v);

  void setBody(const std::string& v);

  void setReason(const std::string& v);

  void setHeaders(const MapType& v);

  bool isClose() const { return m_close; }

  /**
     * @brief 设置是否自动关闭
     */
  void setClose(bool v);

  bool isChunk() const;

  void setChunk(bool v);

  /**
     * @brief 是否websocket
     */
  bool isWebsocket() const { return m_websocket; }

  /**
     * @brief 设置是否websocket
     */
  void setWebsocket(bool v) { m_websocket = v; }

  /**
     * @brief 获取响应头部参数
     * @param[in] key 关键字
     * @param[in] def 默认值
     * @return 如果存在返回对应值,否则返回def
     */
  std::string getHeader(const std::string& key,
                        const std::string& def = "") const;

  /**
     * @brief 设置响应头部参数
     * @param[in] key 关键字
     * @param[in] val 值
     */
  void setHeader(const std::string& key, const std::string& val);

  /**
     * @brief 删除响应头部参数
     * @param[in] key 关键字
     */
  void delHeader(const std::string& key);

 public:
  template <class T>
  bool checkGetHeaderAs(const std::string& key, T& val,
                        const T& def = T()) const {
    return checkGetAs(m_headers, key, val, def);
  }

  template <class T>
  T getHeaderAs(const std::string& key, const T& def = T()) const {
    return getAs(m_headers, key, def);
  }

  template <class T>
  void setHeaderAs(const std::string& key, const T& value) {
    setAs(m_headers, key, value);
  }

  template <class T>
  bool checkGetCookieAs(const std::string& key, T& val,
                        const T& def = T()) const {
    return checkGetAs(m_cookies, key, val, def);
  }

  template <class T>
  T getCookieAs(const std::string& key, const T& def = T()) const {
    return getAs(m_cookies, key, def);
  }

  template <class T>
  void setCookieAs(const std::string& key, const T& value) {
    setAs(m_cookies, key, value);
  }

 public:
  void setRedirect(const std::string& uri);

  void setCookie(const std::string& key, const std::string& val,
                 time_t expired = 0, const std::string& path = "",
                 const std::string& domain = "", bool secure = false);

 private:
  // 响应状态
  HttpStatus m_status;
  // 版本
  uint8_t m_version;
  // 是否自动关闭
  bool m_close;
  // 是否为websocket
  bool m_websocket;

  bool m_ischunk;
  // 响应消息体
  std::string m_body;
  // 响应原因
  std::string m_reason;
  // 响应头部MAP
  MapType m_headers;

  std::vector<std::string> m_cookies;
};

}  // namespace http

}  // namespace ddg

#endif
