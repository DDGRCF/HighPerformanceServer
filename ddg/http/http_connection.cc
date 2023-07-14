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
  uint64_t buff_size = HttpRequestParser::GetHttpRequestBufferSize();
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
      //len -= 2;

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
        (int)HttpResult::Error::SEND_CLOSE_BY_PEER, nullptr,
        "send request closed by peer: " + addr->toString());
  }

  // 如果小于零，就是发生其他错误
  if (ret < 0) {
    return std::make_shared<HttpResult>(
        (int)HttpResult::Error::SEND_SOCKET_ERROR, nullptr,
        "send request socket error errno=" + std::to_string(errno) +
            " errstr=" + std::string(strerror(errno)));
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

}  // namespace http

}  // namespace ddg
