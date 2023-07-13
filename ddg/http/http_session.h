#ifndef DDG_HTTP_SESSION_H_
#define DDG_HTTP_SESSION_H_

#include "ddg/http/http.h"
#include "ddg/stream/socket_stream.h"

namespace ddg {

namespace http {
class HttpSession : public SocketStream {
 public:
  using ptr = std::shared_ptr<HttpSession>;

  HttpSession(Socket::ptr sock, bool owner = true);

  HttpRequest::ptr recvRequest();

  int sendResponse(HttpResponse::ptr rsp);
};
}  // namespace http

}  // namespace ddg

#endif
