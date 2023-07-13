#ifndef DDG_HTTP_HTTP_SERVER_H_
#define DDG_HTTP_HTTP_SERVER_H_

#include "ddg/http/http_session.h"
#include "ddg/http/servlet.h"
#include "ddg/tcp_server.h"

namespace ddg {
namespace http {

class HttpServer : public TcpServer {
 public:
  using ptr = std::shared_ptr<HttpServer>;

  HttpServer(bool keepalive = false,
             ddg::IOManager* worker = ddg::IOManager::GetThis(),
             ddg::IOManager* io_worker = ddg::IOManager::GetThis(),
             ddg::IOManager* accept_worker = ddg::IOManager::GetThis());

  ServletDispatch::ptr getServletDispatch() const;

  void setServletDispatch(ServletDispatch::ptr v);

  virtual void setName(const std::string& v) override;

 protected:
  virtual void handleClient(Socket::ptr client) override;

 private:
  bool m_iskeepalive;

  ServletDispatch::ptr m_dispatch;
};

}  // namespace http
}  // namespace ddg

#endif
