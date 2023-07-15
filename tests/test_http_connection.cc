#include "ddg/http/http_connection.h"
#include "ddg/iomanager.h"

static ddg::Logger::ptr g_logger = DDG_LOG_ROOT();

void test_pool() {
  ddg::http::HttpConnectionPool::ptr pool(new ddg::http::HttpConnectionPool(
      "www.sylar.top", "", 80, false, 10, 1000 * 30, 5));
  ddg::IOManager::GetThis()->addTimer(1000, [pool] {
    auto r = pool->doGet("/blog/", 300);
    DDG_LOG_DEBUG(g_logger) << r->toString();
  });
}

void run() {
  ddg::Address::ptr addr = ddg::Address::LookupAnyIPAddress("www.sylar.top:80");
  if (!addr) {
    DDG_LOG_DEBUG(g_logger) << "get addr error";
  }

  ddg::Socket::ptr sock = ddg::Socket::CreateTcp(addr);
  if (!sock->connect(addr)) {
    DDG_LOG_DEBUG(g_logger) << "connect " << *addr << " failed";
    return;
  }

  ddg::http::HttpConnection::ptr conn(new ddg::http::HttpConnection(sock));
  ddg::http::HttpRequest::ptr req(new ddg::http::HttpRequest);
  req->setPath("/blog/");
  req->setHeader("host", "www.sylar.top");
  DDG_LOG_DEBUG(g_logger) << "req: " << std::endl << *req;
  conn->sendRequest(req);
  auto rsp = conn->recvResponse();

  if (!rsp) {
    DDG_LOG_DEBUG(g_logger) << "recv response error";
    return;
  }

  DDG_LOG_DEBUG(g_logger) << "rsp: " << *rsp;
}

int main() {
  ddg::IOManager iom(2);
  iom.schedule(run);
  iom.schedule(test_pool);
  iom.start();
  iom.stop();
  return 0;
}
