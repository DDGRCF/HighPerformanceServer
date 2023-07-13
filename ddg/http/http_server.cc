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
  do {
    auto req = session->recvRequest();
    if (!req) {
      DDG_LOG_DEBUG(g_logger)
          << "recv http request fail, errno=" << errno
          << " errstr=" << strerror(errno) << " cliet:" << *client
          << " keep_alive=" << m_iskeepalive;
      break;
    }

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
