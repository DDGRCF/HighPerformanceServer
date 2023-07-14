#include "ddg/http/http_server.h"
#include "ddg/http/servlets/config_servlet.h"

#include "ddg/log.h"

namespace ddg {
namespace http {

static ddg::Logger::ptr g_logger = DDG_LOG_ROOT();

HttpServer::HttpServer(bool keepalive, ddg::IOManager* worker,
                       ddg::IOManager* io_worker, ddg::IOManager* accept_worker)
    : TcpServer(worker, io_worker, accept_worker), m_iskeepalive(keepalive) {
  m_dispatch.reset(new ServletDispatch);

  m_type = "http";
  m_dispatch->addServlet("/_/config", Servlet::ptr(new servlet::ConfigServlet));
}

void HttpServer::setName(const std::string& v) {
  TcpServer::setName(v);
  m_dispatch->setDefault(std::make_shared<NotFoundServlet>(v));
}

void HttpServer::handleClient(Socket::ptr client) {
  DDG_LOG_DEBUG(g_logger) << "handleClient " << *client;
  HttpSession::ptr session(new HttpSession(client));
  // 循环每次接收request，然后设置响应，最后将请求和响应参数交给dispatch进行分发
  do {
    auto req = session->recvRequest();
    if (!req) {
      DDG_LOG_DEBUG(g_logger)
          << "recv http request fail, errno=" << errno
          << " errstr=" << strerror(errno) << " cliet:" << *client
          << " keep_alive=" << m_iskeepalive;
      break;
    }

    // 两种情况close 1. 请求close 2. 服务器不支持keepalive
    HttpResponse::ptr rsp(
        new HttpResponse(req->getVersion(), req->isClose() || !m_iskeepalive));
    rsp->setHeader("Server", getName());
    m_dispatch->handle(req, rsp, session);
    session->sendResponse(rsp);

    if (!m_iskeepalive || req->isClose()) {
      break;
    }
  } while (true);
  session->close();
}

}  // namespace http

}  // namespace ddg
