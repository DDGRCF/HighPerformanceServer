#include "ddg/http/http.h"

#include <sstream>

#include "ddg/utils.h"

namespace ddg {
namespace http {

HttpMethod StringToHttpMethod(const std::string& m) {
#define XX(num, name, string) \
  if (m == #string) {         \
    return HttpMethod::name;  \
  }
  HTTP_METHOD_MAP(XX);
#undef XX
  return HttpMethod::INVALID_METHOD;
}

HttpMethod CharsToHttpMethod(const char* m) {
#define XX(num, name, string)                      \
  if (strncmp(#string, m, strlen(#string)) == 0) { \
    return HttpMethod::name;                       \
  }
  HTTP_METHOD_MAP(XX);
#undef XX
  return HttpMethod::INVALID_METHOD;
}  // namespace http

const char* HttpMethodToString(const HttpMethod& m) {
  switch (m) {
#define XX(num, name, string) \
  case HttpMethod::name: {    \
    return #string;           \
    break;                    \
  }
    HTTP_METHOD_MAP(XX)
#undef XX
    default:
      return "UNKNOWN";
  }
}

const char* HttpStatusToString(const HttpStatus& s) {
  switch (s) {
#define XX(code, name, msg) \
  case HttpStatus::name: {  \
    return #msg;            \
  }
    HTTP_STATUS_MAP(XX);
    default:
      return "UNKNOWN";
  }
}

bool CaseInsensitiveLess::operator()(const std::string& lhs,
                                     const std::string& rhs) const {
  return strcasecmp(lhs.c_str(), rhs.c_str()) < 0;
}

HttpRequest::HttpRequest(uint8_t version, bool close)
    : m_method(HttpMethod::GET),
      m_version(version),
      m_close(close),
      m_websocket(false),
      m_parser_paramflag(0),
      m_path("/") {}

std::shared_ptr<HttpResponse> HttpRequest::createResponse() {
  HttpResponse::ptr resp =
      std::make_shared<HttpResponse>(getVersion(), isClose());
  return resp;
}

uint8_t HttpRequest::getVersion() const {
  return m_version;
}

const std::string& HttpRequest::getPath() const {
  return m_path;
}

const std::string& HttpRequest::getQuery() const {
  return m_query;
}

const std::string& HttpRequest::getBody() const {
  return m_body;
}

const HttpRequest::MapType& HttpRequest::getHeaders() const {
  return m_headers;
}

const HttpRequest::MapType& HttpRequest::getParams() const {
  return m_params;
}

const HttpRequest::MapType& HttpRequest::getCookies() const {
  return m_cookies;
}

void HttpRequest::setVersion(uint8_t v) {
  m_version = v;
}

void HttpRequest::setPath(const std::string& v) {
  m_path = v;
}

void HttpRequest::setQuery(const std::string& v) {
  m_query = v;
}

void HttpRequest::setFragment(const std::string& v) {
  m_fragment = v;
}

void HttpRequest::setBody(const std::string& v) {
  if (!v.empty()) {
    setHeaderAs("content-length", v.size());
  }
  m_body = v;
}

void HttpRequest::setMethod(HttpMethod v) {
  m_method = v;
}

void HttpRequest::setHeaders(const MapType& v) {
  m_headers = v;
}

void HttpRequest::setParams(const MapType& v) {
  m_params = v;
}

void HttpRequest::setCookies(const MapType& v) {
  m_cookies = v;
}

void HttpRequest::setClose(bool v) {
  m_close = v;
}

std::string HttpRequest::getHeader(const std::string& key,
                                   const std::string& def) {
  auto it = m_headers.find(key);
  return it == m_headers.end() ? def : it->second;
}

std::string HttpRequest::getParam(const std::string& key,
                                  const std::string& def) {
  initCookies();
  auto it = m_cookies.find(key);
  return it == m_cookies.end() ? def : it->second;
}

std::string HttpRequest::getCookie(const std::string& key,
                                   const std::string& def) {
  initCookies();
  auto it = m_cookies.find(key);
  return it == m_cookies.end() ? def : it->second;
}

void HttpRequest::setHeader(const std::string& key, const std::string& val) {
  m_headers[key] = val;
}

void HttpRequest::setParam(const std::string& key, const std::string& val) {
  m_params[key] = val;
}

void HttpRequest::setCookie(const std::string& key, const std::string& val) {
  m_cookies[key] = val;
}

void HttpRequest::delHeader(const std::string& key) {
  m_headers.erase(key);
}

void HttpRequest::delParam(const std::string& key) {
  m_params.erase(key);
}

void HttpRequest::delCookie(const std::string& key) {
  m_headers.erase(key);
}

bool HttpRequest::hasHeader(const std::string& key, std::string* val) const {
  auto it = m_headers.find(key);
  if (it == m_headers.end()) {
    return false;
  }
  if (val) {
    *val = it->second;
  }
  return true;
}

bool HttpRequest::hasParam(const std::string& key, std::string* val) const {
  auto it = m_params.find(key);
  if (it == m_params.end()) {
    return false;
  }
  if (val) {
    *val = it->second;
  }
  return true;
}

bool HttpRequest::hasCookie(const std::string& key, std::string* val) const {
  auto it = m_cookies.find(key);
  if (it == m_cookies.end()) {
    return false;
  }
  if (val) {
    *val = it->second;
  }
  return true;
}

bool HttpRequest::isClose() const {
  return m_close;
}

bool HttpRequest::isWebsocket() const {
  return m_websocket;
}

std::ostream& HttpRequest::dump(std::ostream& os) const {
  os << HttpMethodToString(m_method) << " " << m_path
     << (m_query.empty() ? "" : "?") << m_query
     << (m_fragment.empty() ? "" : "#") << m_fragment << " HTTP/"
     << static_cast<uint32_t>(m_version >> 4) << "."
     << static_cast<uint32_t>(m_version & 0x0f) << "\r\n";
  if (!m_websocket) {
    os << "connection: " << (m_close ? "close" : "keep-alive") << "\r\n";
  }

  for (auto& elem : m_headers) {
    if (!m_websocket && strcasecmp(elem.first.c_str(), "connection") == 0) {
      continue;
    }  // TODO:
    os << elem.first << ": " << elem.second << "\r\n";
  }

  os << "\r\n";

  if (!m_body.empty()) {
    os << m_body;
  }

  return os;
}

void HttpRequest::init() {
  std::string conn = getHeader("connection");
  if (!conn.empty()) {
    if (strcasecmp(conn.c_str(), "keep-alive") == 0) {
      m_close = false;
    } else {
      m_close = true;
    }
  }
}

void HttpRequest::initParam() {
  initQueryParam();
  initBodyParam();
  initCookies();
}

void HttpRequest::initQueryParam() {  // TODO:
  if (m_parser_paramflag & 0x01) {
    return;
  }
#define PARSE_PARAM(str, m, flag, trim)                                    \
  size_t pos = 0;                                                          \
  do {                                                                     \
    size_t last = pos;                                                     \
    pos = str.find('=', pos);                                              \
    if (pos == std::string::npos) {                                        \
      break;                                                               \
    }                                                                      \
    size_t key = pos;                                                      \
    pos = str.find(flag, pos);                                             \
    m.insert(                                                              \
        {trim(str.substr(last, key - last)),                               \
         ddg::StringUtil::UrlDecode(str.substr(key + 1, pos - key - 1))}); \
    if (pos == std::string::npos) {                                        \
      break;                                                               \
    }                                                                      \
    pos++;                                                                 \
  } while (true);
  PARSE_PARAM(m_query, m_params, '&', );
  m_parser_paramflag |= 0x01;
}

void HttpRequest::initBodyParam() {  // TODO:
  if (m_parser_paramflag & 0x02) {
    return;
  }
  std::string content_type = getHeader("content-type");
  if (strcasecmp(content_type.c_str(), "application/x-www-form-urlencoded") ==
      0) {
    m_parser_paramflag |= 0x02;
    return;
  }
  PARSE_PARAM(m_body, m_params, '&', );
  m_parser_paramflag |= 0x02;
}

void HttpRequest::initCookies() {  // TODO:
  if (m_parser_paramflag & 0x04) {
    return;
  }
  std::string cookie = getHeader("cookie");
  if (cookie.empty()) {
    m_parser_paramflag |= 0x04;
    return;
  }
  PARSE_PARAM(cookie, m_cookies, ';', ddg::StringUtil::Trim);
  m_parser_paramflag |= 0x04;
}

std::ostream& operator<<(std::ostream& os, const HttpRequest& req) {
  return req.dump(os);
}

std::string HttpRequest::toString() const {
  std::stringstream ss;
  dump(ss);
  return ss.str();
}

HttpResponse::HttpResponse(uint8_t version, bool close, bool chunk)
    : m_status(HttpStatus::OK),
      m_version(version),
      m_close(close),
      m_websocket(false),
      m_ischunk(chunk) {}

std::string HttpResponse::getHeader(const std::string& key,
                                    const std::string& def) const {
  auto it = m_headers.find(key);
  return it == m_headers.end() ? def : it->second;
}

HttpStatus HttpResponse::getStatus() const {
  return m_status;
}

uint8_t HttpResponse::getVersion() const {
  return m_version;
}

const std::string& HttpResponse::getBody() const {
  return m_body;
}

const std::string& HttpResponse::getReason() const {
  return m_reason;
}

const HttpResponse::MapType& HttpResponse::getHeaders() const {
  return m_headers;
}

void HttpResponse::setStatus(HttpStatus v) {
  m_status = v;
}

void HttpResponse::setVersion(uint8_t v) {
  m_version = v;
}

void HttpResponse::setBody(const std::string& v) {
  if (!m_ischunk && !v.empty()) {
    setHeaderAs("content-length", v.size());
  }
  m_body = v;
}

void HttpResponse::setReason(const std::string& v) {
  m_reason = v;
}

void HttpResponse::setHeaders(const MapType& v) {
  m_headers = v;
}

void HttpResponse::setClose(bool v) {
  m_close = v;
}

bool HttpResponse::isChunk() const {
  return m_ischunk;
}

void HttpResponse::setChunk(bool v) {
  if (v) {
    delHeader("content-length");
  }
  m_ischunk = v;
}

void HttpResponse::setHeader(const std::string& key, const std::string& val) {
  m_headers[key] = val;
}

void HttpResponse::delHeader(const std::string& key) {
  m_headers.erase(key);
}

// 这种方法流浪器主导的重定向
void HttpResponse::setRedirect(const std::string& uri) {
  m_status = HttpStatus::FOUND;
  setHeader("location", uri);
}

void HttpResponse::setCookie(const std::string& key, const std::string& val,
                             time_t expired, const std::string& path,
                             const std::string& domain, bool secure) {
  std::stringstream ss;
  ss << key << "=" << val;
  if (expired > 0) {
    ss << ";expires" << StringUtil::Time2Str(expired, "%a, %d %b %Y %H:%M:%S")
       << " GMT";
  }

  if (!domain.empty()) {
    ss << ";domain=" << domain;
  }
  if (!path.empty()) {
    ss << ";path=" << path;
  }
  if (secure) {
    ss << ";secure";
  }
  m_cookies.push_back(ss.str());
}

std::ostream& HttpResponse::dump(std::ostream& os) const {
  os << "HTTP/" << static_cast<uint32_t>(m_version >> 4) << "."
     << static_cast<uint32_t>(m_version & 0x0F) << " "
     << static_cast<uint32_t>(m_status) << " "
     << (m_reason.empty() ? HttpStatusToString(m_status) : m_reason) << "\r\n";

  for (auto& elem : m_headers) {
    if (!m_websocket && strcasecmp(elem.first.c_str(), "connection") == 0) {
      continue;
    }
    os << elem.first << ": " << elem.second << "\r\n";
  }

  for (auto& elem : m_cookies) {
    os << "Set-Cookie: " << elem << "\r\n";
  }
  if (!m_websocket) {
    os << "connection: " << (m_close ? "close" : "keep-alive") << "\r\n";
  }

  os << "\r\n";
  if (!m_body.empty()) {
    os << m_body;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const HttpResponse& resp) {
  return resp.dump(os);
}

std::string HttpResponse::toString() const {
  std::stringstream ss;
  dump(ss);
  return ss.str();
}

}  // namespace http
}  // namespace ddg
