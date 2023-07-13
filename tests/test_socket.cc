#include <iostream>
#include <string>

#include "ddg/address.h"
#include "ddg/iomanager.h"
#include "ddg/socket.h"

static ddg::Logger::ptr g_logger = DDG_LOG_ROOT();

void test_socket() {
  ddg::IPAddress::ptr addr = ddg::Address::LookupAnyIPAddress("www.baidu.com");
  if (!addr) {
    DDG_LOG_ERROR(g_logger) << "get address fail!";
    return;
  }
  addr->setPort(80);
  DDG_LOG_DEBUG(g_logger) << "get address: " << *addr;

  ddg::Socket::ptr sock = ddg::Socket::CreateTcp(addr);
  DDG_LOG_DEBUG(g_logger) << "addr = " << *addr;
  if (!sock->connect(addr, 1000)) {
    DDG_LOG_ERROR(g_logger) << "connect addr = " << *addr << "fail";
    return;
  }

  DDG_LOG_DEBUG(g_logger) << "connect to " << *addr;
  DDG_LOG_DEBUG(g_logger) << "local connect: " << *sock->getLocalAddress();
  DDG_LOG_DEBUG(g_logger) << "remote connect: " << *sock->getRemoteAddress();

  const char buf[] = "GET / HTTP/1.0\r\n\r\n";
  int ret = sock->send(buf, sizeof(buf));
  if (ret <= 0) {
    DDG_LOG_ERROR(g_logger) << "Socket::send fail ret = " << ret;
    return;
  }

  std::string bufs;
  bufs.resize(4096);
  ret = sock->recv(&bufs[0], bufs.size());

  if (ret <= 0) {
    DDG_LOG_ERROR(g_logger) << "recv fail ret = " << ret;
    return;
  }
  bufs.resize(ret);
  DDG_LOG_DEBUG(g_logger) << bufs;
}

void test_socketiomanager() {
  auto func = []() {
    ddg::IPAddress::ptr addr =
        ddg::Address::LookupAnyIPAddress("www.baidu.com:80");
    if (!addr) {
      DDG_LOG_ERROR(g_logger) << "get address fail!";
      return;
    }
    DDG_LOG_DEBUG(g_logger) << "get address: " << *addr;

    ddg::Socket::ptr sock = ddg::Socket::CreateTcp(addr);
    if (!sock->connect(addr, 1000)) {
      DDG_LOG_ERROR(g_logger) << "connect addr = " << *addr << "fail";
      return;
    }
    DDG_LOG_DEBUG(g_logger) << "connect to " << *addr;
    DDG_LOG_DEBUG(g_logger) << "local connect: " << *sock->getLocalAddress();
    DDG_LOG_DEBUG(g_logger) << "remote connect: " << *sock->getRemoteAddress();
  };
  ddg::IOManager iom;
  iom.start();
  iom.schedule(func);
  iom.stop();
}

int main() {
  std::cout << std::string(150, '*') << std::endl;
  test_socket();
  std::cout << std::string(150, '*') << std::endl;
  test_socketiomanager();
  return 0;
}
