#include "ddg/http/http_session.h"

#include "ddg/http/http_parser.h"

namespace ddg {
namespace http {

HttpSession::HttpSession(Socket::ptr sock, bool owner)
    : SocketStream(sock, owner) {}

// 总体的思路就是一直解析，然后将数据送入，如果中间有部分数据没有解析就跳过
// 如果解析完毕后最后的未解析数据就是body
HttpRequest::ptr HttpSession::recvRequest() {
  HttpRequestParser::ptr parser = std::make_shared<HttpRequestParser>();
  // 这个大小是能接受的最大大小
  uint64_t buff_size = HttpRequestParser::GetHttpRequestBufferSize();
  std::shared_ptr<char> buffer(new char[buff_size],
                               [](char* ptr) { delete[] ptr; });
  char* data = buffer.get();
  int offset = 0;
  do {  // TODO:
    int len = read(data + offset, buff_size - offset);
    if (len <= 0) {
      close();
      return nullptr;
    }

    len += offset;
    // 有部分没有解析掉，就把这部分数据跳过去
    size_t nparse = parser->execute(data, len);
    if (parser->getError()) {
      close();
      return nullptr;
    }

    offset = len - nparse;  // TODO:
    if (offset == static_cast<int>(buff_size)) {
      close();
      return nullptr;
    }

    if (parser->isFinished()) {
      break;
    }
  } while (true);

  int64_t length = parser->getContentLength();
  if (length > 0) {
    std::string body;
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
    parser->getData()->setBody(body);
  }

  parser->getData()->init();
  return parser->getData();
}

int HttpSession::sendResponse(HttpResponse::ptr rsp) {
  std::stringstream ss;
  ss << *rsp;
  std::string data = ss.str();
  return writeFixSize(data.c_str(), data.size());
}

}  // namespace http
}  // namespace ddg
