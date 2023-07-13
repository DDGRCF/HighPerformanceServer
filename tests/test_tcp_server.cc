#include <memory>

#include "ddg/iomanager.h"
#include "ddg/log.h"
#include "ddg/tcp_server.h"

static ddg::Logger::ptr g_logger = DDG_LOG_ROOT();

int run() {
  auto addr = ddg::Address::LookupAny("0.0.0.0:8033");
  auto addr2 = std::make_shared<ddg::UnixAddress>("/tmp/unix_addr");
  DDG_LOG_DEBUG(g_logger) << *addr2;
  std::vector<ddg::Address::ptr> addrs;
  addrs.push_back(addr);
  addrs.push_back(addr2);

  ddg::TcpServer::ptr tcp_server = std::make_shared<ddg::TcpServer>();

  std::vector<ddg::Address::ptr> fails;
  while (!tcp_server->bind(addrs, fails)) {
    addrs = fails;
    sleep(2);
  }

  tcp_server->start();

  return 0;
}

int main() {
  ddg::IOManager iom(2);
  iom.start();
  iom.schedule(run);
  iom.stop();
  return 0;
}
