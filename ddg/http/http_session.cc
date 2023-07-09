#include "ddg/http/http_session.h"

#include "ddg/http/http_parser.h"

namespace ddg {
namespace http {

HttpSession::HttpSession(Socket::ptr sock, bool owner)
    : SocketStream(sock, owner) {}

HttpRequest::ptr HttpSession::recvRequest() {
  HttpRequestParser::ptr parser = std::make_shared<HttpRequestParser>();
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
    size_t nparse = parser->execute(data, len, true);
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
