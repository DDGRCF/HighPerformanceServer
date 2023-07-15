#include "ddg/http/http_connection.h"

#include "ddg/http/http.h"
#include "ddg/http/http_parser.h"
#include "ddg/socket.h"

namespace ddg {
namespace http {

std::string HttpResult::toString() const {
  std::stringstream ss;
  ss << "[HttpResult result = " << result << " error = " << error
     << " response = " << (response ? response->toString() : "nullptr") << "]";
  return ss.str();
}

std::ostream& operator<<(std::ostream& os, const HttpResult& result) {
  os << result.toString();
  return os;
}

HttpResult::HttpResult(int _result, HttpResponse::ptr _response,
                       const std::string& _error)
    : result(_result), response(_response), error(_error) {}

HttpResult::~HttpResult() {}

HttpConnection::HttpConnection(Socket::ptr sock, bool owner)
    : SocketStream(sock, owner) {}

HttpConnection::~HttpConnection() {}

HttpResponse::ptr HttpConnection::recvResponse() {
  HttpResponseParser::ptr parser(new HttpResponseParser);
  size_t buff_size = HttpRequestParser::GetHttpRequestBufferSize();
  std::shared_ptr<char> buffer(new char[buff_size + 1],
                               [](char* ptr) { delete[] ptr; });
  char* data = buffer.get();
  int offset = 0;
  do {
    int len = read(data + offset, buff_size - offset);
    if (len <= 0) {
      close();
      return nullptr;
    }
    len += offset;
    data[len] = '\0';
    size_t nparse = parser->execute(data, len, false);
    if (parser->getError()) {
      close();
      return nullptr;
    }
    offset = len - nparse;
    if (offset == (int)buff_size) {
      close();
      return nullptr;
    }
    if (parser->isFinished()) {
      break;
    }
  } while (true);
  auto& client_parser = parser->getParser();
  std::string body;
  if (client_parser.chunked) {
    int len = offset;
    do {
      bool begin = true;
      do {
        if (!begin || len == 0) {
          int rt = read(data + len, buff_size - len);
          if (rt <= 0) {
            close();
            return nullptr;
          }
          len += rt;
        }
        data[len] = '\0';
        size_t nparse = parser->execute(data, len, true);
        if (parser->getError()) {
          close();
          return nullptr;
        }
        len -= nparse;
        if (len == (int)buff_size) {
          close();
          return nullptr;
        }
        begin = false;
      } while (!parser->isFinished());

      DDG_LOG_DEBUG(g_logger) << "content_len=" << client_parser.content_len;
      if (client_parser.content_len + 2 <= len) {
        body.append(data, client_parser.content_len);
        memmove(data, data + client_parser.content_len + 2,
                len - client_parser.content_len - 2);
        len -= client_parser.content_len + 2;
      } else {
        body.append(data, len);
        int left = client_parser.content_len - len + 2;
        while (left > 0) {
          int rt = read(data, left > (int)buff_size ? (int)buff_size : left);
          if (rt <= 0) {
            close();
            return nullptr;
          }
          body.append(data, rt);
          left -= rt;
        }
        body.resize(body.size() - 2);
        len = 0;
      }
    } while (!client_parser.chunks_done);
  } else {
    int64_t length = parser->getContentLength();
    if (length > 0) {
      body.resize(length);

      int len = 0;
      if (length >= offset) {
        memcpy(&body[0], data, offset);
        len = offset;
      } else {
        memcpy(&body[0], data, length);
        len = length;
      }
      length -= offset;
      if (length > 0) {
        if (readFixSize(&body[len], length) <= 0) {
          close();
          return nullptr;
        }
      }
    }
  }
  if (!body.empty()) {
    auto content_encoding = parser->getData()->getHeader("content-encoding");
    DDG_LOG_DEBUG(g_logger)
        << "content_encoding: " << content_encoding << " size=" << body.size();
    // 不同的编码方式
    // if (strcasecmp(content_encoding.c_str(), "gzip") == 0) {
    // auto zs = ZlibStream::CreateGzip(false);
    // zs->write(body.c_str(), body.size());
    // zs->flush();
    // zs->getResult().swap(body);
    // } else if (strcasecmp(content_encoding.c_str(), "deflate") == 0) {
    // auto zs = ZlibStream::CreateDeflate(false);
    // zs->write(body.c_str(), body.size());
    // zs->flush();
    // zs->getResult().swap(body);
    // }
    parser->getData()->setBody(body);
  }
  return parser->getData();
}

int HttpConnection::sendRequest(HttpRequest::ptr rsp) {
  std::stringstream ss;
  ss << *rsp;
  std::string data = ss.str();
  return writeFixSize(data.c_str(), data.size());
}

HttpResult::ptr HttpConnection::DoGet(
    const std::string& url, time_t timeout_ms,
    const std::map<std::string, std::string>& headers,
    const std::string& body) {
  Uri::ptr uri = Uri::Create(url);
  if (uri) {
    return std::make_shared<HttpResult>(
        static_cast<int>(HttpResult::Error::INVALID_URL), nullptr,
        "invalid url: " + url);
  }
  return DoGet(uri, timeout_ms, headers, body);
}

HttpResult::ptr HttpConnection::DoGet(
    Uri::ptr uri, time_t timeout_ms,
    const std::map<std::string, std::string>& headers,
    const std::string& body) {
  return DoRequest(HttpMethod::GET, uri, timeout_ms, headers, body);
}

HttpResult::ptr HttpConnection::DoPost(
    const std::string& url, time_t timeout_ms,
    const std ::map<std::string, std::string>& headers,
    const std::string& body) {
  Uri::ptr uri = Uri::Create(url);
  if (uri) {
    return std::make_shared<HttpResult>(
        static_cast<int>(HttpResult::Error::INVALID_URL), nullptr,
        "invalid url: " + url);
  }
  return DoPost(uri, timeout_ms, headers, body);
}

HttpResult::ptr HttpConnection::DoPost(
    Uri::ptr uri, time_t timeout_ms,
    const std ::map<std::string, std::string>& headers,
    const std::string& body) {
  return DoRequest(HttpMethod::POST, uri, timeout_ms, headers, body);
}

HttpResult::ptr HttpConnection::DoRequest(
    HttpMethod method, const std::string& url, time_t timeout_ms,
    const std::map<std::string, std::string>& headers,
    const std::string& body) {

  Uri::ptr uri = Uri::Create(url);
  if (uri) {
    return std::make_shared<HttpResult>(
        static_cast<int>(HttpResult::Error::INVALID_URL), nullptr,
        "invalid url: " + url);
  }
  return DoRequest(method, uri, timeout_ms, headers, body);
}

HttpResult::ptr HttpConnection::DoRequest(
    HttpMethod method, Uri::ptr uri, time_t timeout_ms,
    const std::map<std::string, std::string>& headers,
    const std::string& body) {
  HttpRequest::ptr req = std::make_shared<HttpRequest>();
  req->setPath(uri->getPath());
  req->setQuery(uri->getQuery());
  req->setFragment(uri->getFragment());
  req->setMethod(method);

  bool has_host = false;
  for (auto& header : headers) {
    if (strcasecmp(header.first.c_str(), "connection") == 0) {
      if (strcasecmp(header.second.c_str(), "keep-alive") == 0) {
        req->setClose(false);
      }
      continue;
    }

    // 这里查看是否找到host，如果找到host就进行设定
    if (!has_host && strcasecmp(header.first.c_str(), "host") == 0) {
      has_host = !header.second.empty();
    }
    req->setHeader(header.first, header.second);
  }

  if (!has_host) {
    req->setHeader("Host", uri->getHost());
  }

  req->setBody(body);
  return DoRequest(req, uri, timeout_ms);
}

HttpResult::ptr HttpConnection::DoRequest(HttpRequest::ptr req, Uri::ptr uri,
                                          time_t timeout_ms) {
  bool is_ssl = uri->getScheme() == "https";
  Address::ptr addr = uri->createAddress();
  if (!addr) {
    return std::make_shared<HttpResult>((int)HttpResult::Error::INVALID_HOST,
                                        nullptr,
                                        "invalid host: " + uri->getHost());
  }
  Socket::ptr sock =
      is_ssl ? SSLSocket::CreateTcp(addr) : Socket::CreateTcp(addr);
  if (!sock) {
    return std::make_shared<HttpResult>(
        (int)HttpResult::Error::CREATE_SOCKET_ERROR, nullptr,
        "create socket fail: " + addr->toString() + " errno=" +
            std::to_string(errno) + " errstr=" + std::string(strerror(errno)));
  }
  if (!sock->connect(addr)) {
    return std::make_shared<HttpResult>((int)HttpResult::Error::CONNECT_FAIL,
                                        nullptr,
                                        "connect fail: " + addr->toString());
  }
  sock->setRecvTimeout(timeout_ms);
  HttpConnection::ptr conn = std::make_shared<HttpConnection>(sock);
  // 如果等于0，就是服务器半关闭，禁止发送数据
  int ret = conn->sendRequest(req);
  if (ret == 0) {
    return std::make_shared<HttpResult>(
        static_cast<int>(HttpResult::Error::WRITE_BUFFER_FULL), nullptr,
        "write buffer full");
  }

  // 如果小于零，就是发生其他错误
  if (ret < 0) {
    return std::make_shared<HttpResult>(
        (int)HttpResult::Error::SEND_SOCKET_ERROR, nullptr,
        "send request socket error errno = " + std::to_string(errno) +
            " errstr = " + std::string(strerror(errno)));
  }

  // 发送响应
  auto rsp = conn->recvResponse();
  if (!rsp) {
    return std::make_shared<HttpResult>(
        (int)HttpResult::Error::TIMEOUT, nullptr,
        "recv response timeout: " + addr->toString() +
            " timeout_ms:" + std::to_string(timeout_ms));
  }
  return std::make_shared<HttpResult>(static_cast<int>(HttpResult::Error::OK),
                                      rsp, "ok");
}

HttpConnectionPool::HttpConnectionPool(const std::string& host,
                                       const std::string& vhost, uint16_t port,
                                       bool is_https, size_t max_size,
                                       time_t max_alive_time,
                                       size_t max_request)
    : m_host(host),
      m_vhost(vhost),
      m_port(port ? port : (is_https ? 443 : 80)),
      m_maxsize(max_size),
      m_maxalivetime(max_alive_time),
      m_maxrequest(max_request),
      m_ishttps(is_https) {}

HttpConnectionPool::~HttpConnectionPool() {}

HttpConnection::ptr HttpConnectionPool::getConnection() {
  time_t now_ms = ddg::GetCurrentMilliSecond();
  std::vector<HttpConnection*> invalid_conns;
  HttpConnection* ptr = nullptr;
  MutexType::Lock lock(m_mutex);
  while (!m_conns.empty()) {
    auto conn = *m_conns.begin();
    m_conns.pop_front();
    if (!conn->isConnected()) {
      invalid_conns.push_back(conn);
      continue;
    }

    if (conn->m_createtime + m_maxalivetime > now_ms) {
      invalid_conns.push_back(conn);
      continue;
    }
    ptr = conn;
    break;
  }
  lock.unlock();
  for (auto elem : invalid_conns) {
    delete elem;
  }
  m_total -= static_cast<int32_t>(invalid_conns.size());
  if (!ptr) {
    IPAddress::ptr addr = Address::LookupAnyIPAddress(m_host);
    if (!addr) {
      DDG_LOG_ERROR(g_logger)
          << "HttpConnectionPool::getConnection error, get addr fail: "
          << m_host;
      return nullptr;
    }
    addr->setPort(m_port);
    Socket::ptr sock =
        m_ishttps ? SSLSocket::CreateTcp(addr) : Socket::CreateTcp(addr);
    if (!sock) {
      DDG_LOG_ERROR(g_logger)
          << "HttpConnectionPool::getConnection error, create sock fail: "
          << *addr;
      return nullptr;
    }

    if (!sock->connect(addr)) {
      DDG_LOG_DEBUG(g_logger)
          << "HttpConnectionPool::getConnection error, sock connect fail: "
          << *addr;
      return nullptr;
    }

    ptr = new HttpConnection(sock);
    m_total++;
  }
  return HttpConnection::ptr(ptr, std::bind(&HttpConnectionPool::ReleasePtr,
                                            std::placeholders::_1, this));
}

// 如果当前ptr的连接关闭，且最大存活时间大于当前时间，且当前请求完毕数量最大请求数量，释放
void HttpConnectionPool::ReleasePtr(HttpConnection* ptr,
                                    HttpConnectionPool* pool) {
  ptr->m_request++;
  if (!ptr->isConnected() ||
      ptr->m_createtime + pool->m_maxalivetime >=
          ddg::GetCurrentMilliSecond() ||
      ptr->m_request >= pool->m_maxrequest ||
      pool->m_total > static_cast<int32_t>(pool->m_maxsize)) {  // TODO:
    delete ptr;
    pool->m_total--;
    return;
  }
  MutexType::Lock lock(pool->m_mutex);
  pool->m_conns.push_back(ptr);
}

HttpResult::ptr HttpConnectionPool::doGet(
    const std::string& url, time_t timeout_ms,
    const std::map<std::string, std::string>& headers,
    const std::string& body) {
  return doRequest(HttpMethod::GET, url, timeout_ms, headers, body);
}

HttpResult::ptr HttpConnectionPool::doGet(
    Uri::ptr uri, time_t timeout_ms,
    const std::map<std::string, std::string>& headers,
    const std::string& body) {
  std::stringstream ss;
  ss << uri->getPath() << (uri->getQuery().empty() ? "" : "?")
     << uri->getQuery() << (uri->getFragment().empty() ? "" : "#")
     << uri->getFragment();
  return doGet(ss.str(), timeout_ms, headers, body);
}

HttpResult::ptr HttpConnectionPool::doPost(
    const std::string& url, time_t timeout_ms,
    const std::map<std::string, std::string>& headers,
    const std::string& body) {
  return doRequest(HttpMethod::POST, url, timeout_ms, headers, body);
}

HttpResult::ptr HttpConnectionPool::doPost(
    Uri::ptr uri, time_t timeout_ms,
    const std::map<std::string, std::string>& headers,
    const std::string& body) {
  std::stringstream ss;
  ss << uri->getPath() << (uri->getQuery().empty() ? "" : "?")
     << uri->getQuery() << (uri->getFragment().empty() ? "" : "#")
     << uri->getFragment();
  return doPost(ss.str(), timeout_ms, headers, body);
}

HttpResult::ptr HttpConnectionPool::doRequest(
    HttpMethod method, const std::string& url, time_t timeout_ms,
    const std::map<std::string, std::string>& headers,
    const std::string& body) {
  HttpRequest::ptr req = std::make_shared<HttpRequest>();
  req->setPath(url);
  req->setMethod(method);
  req->setClose(false);
  bool has_host = false;
  for (auto& i : headers) {
    if (strcasecmp(i.first.c_str(), "connection") == 0) {
      if (strcasecmp(i.second.c_str(), "keep-alive") == 0) {
        req->setClose(false);
      }
      continue;
    }

    if (!has_host && strcasecmp(i.first.c_str(), "host") == 0) {
      has_host = !i.second.empty();
    }

    req->setHeader(i.first, i.second);
  }
  if (!has_host) {
    if (m_vhost.empty()) {
      req->setHeader("Host", m_host);
    } else {
      req->setHeader("Host", m_vhost);
    }
  }
  req->setBody(body);
  return doRequest(req, timeout_ms);
}

HttpResult::ptr HttpConnectionPool::doRequest(
    HttpMethod method, Uri::ptr uri, time_t timeout_ms,
    const std::map<std::string, std::string>& headers,
    const std::string& body) {
  std::stringstream ss;
  ss << uri->getPath() << (uri->getQuery().empty() ? "" : "?")
     << uri->getQuery() << (uri->getFragment().empty() ? "" : "#")
     << uri->getFragment();
  return doRequest(method, ss.str(), timeout_ms, headers, body);
}

HttpResult::ptr HttpConnectionPool::doRequest(HttpRequest::ptr req,
                                              time_t timeout_ms) {
  auto conn = getConnection();
  if (!conn) {
    return std::make_shared<HttpResult>(
        static_cast<int>(HttpResult::Error::POOL_GET_CONNECTION), nullptr,
        "pool host:" + m_host + " port:" + std::to_string(m_port));
  }
  auto sock = conn->getSocket();
  if (!sock) {
    return std::make_shared<HttpResult>(
        static_cast<int>(HttpResult::Error::POOL_INVALID_CONNECTION), nullptr,
        "pool host:" + m_host + " port:" + std::to_string(m_port));
  }
  sock->setRecvTimeout(timeout_ms);
  int ret = conn->sendRequest(req);
  if (ret == 0) {
    return std::make_shared<HttpResult>(
        static_cast<int>(HttpResult::Error::SEND_CLOSE_BY_PEER), nullptr,
        "send request closed by peer: " + sock->getRemoteAddress()->toString());
  }
  if (ret < 0) {
    return std::make_shared<HttpResult>(
        static_cast<int>(HttpResult::Error::SEND_SOCKET_ERROR), nullptr,
        "send request socket error errno=" + std::to_string(errno) +
            " errstr=" + std::string(strerror(errno)));
  }
  auto rsp = conn->recvResponse();
  if (!rsp) {
    return std::make_shared<HttpResult>(
        static_cast<int>(HttpResult::Error::TIMEOUT), nullptr,
        "recv response timeout: " + sock->getRemoteAddress()->toString() +
            " timeout_ms:" + std::to_string(timeout_ms));
  }
  return std::make_shared<HttpResult>(static_cast<int>(HttpResult::Error::OK),
                                      rsp, "ok");
}

}  // namespace http

}  // namespace ddg
