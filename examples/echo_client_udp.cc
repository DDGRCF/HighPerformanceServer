#include <stdlib.h>
#include "ddg/iomanager.h"
#include "ddg/log.h"
#include "ddg/socket.h"

static ddg::Logger::ptr g_logger = DDG_LOG_ROOT();

const char* ip = nullptr;
uint16_t port = 0;

void run() {
  ddg::IPAddress::ptr addr = ddg::Address::LookupAnyIPAddress(ip);
  if (!addr) {
    DDG_LOG_ERROR(g_logger) << "invalid ip: " << ip;
    return;
  }
  addr->setPort(port);

  ddg::Socket::ptr sock = ddg::Socket::CreateUdp(addr);

  ddg::IOManager::GetThis()->schedule([sock]() {
    ddg::Address::ptr addr(new ddg::IPv4Address);
    DDG_LOG_INFO(g_logger) << "begin recv";
    while (true) {
      char buff[1024];
      int len = sock->recvFrom(buff, 1024, addr);
      if (len > 0) {
        std::cout << std::endl
                  << "recv: " << std::string(buff, len) << " from: " << *addr
                  << std::endl;
      }
    }
  });
  sleep(1);
  while (true) {
    std::string line;
    std::cout << "input> ";
    std::getline(std::cin, line);
    if (!line.empty()) {
      int len = sock->sendTo(line.c_str(), line.size(), addr);
      if (len < 0) {
        int err = sock->getError();
        DDG_LOG_ERROR(g_logger)
            << "send error err=" << err << " errstr=" << strerror(err)
            << " len=" << len << " addr=" << *addr << " sock=" << *sock;
      } else {
        DDG_LOG_INFO(g_logger) << "send " << line << " len:" << len;
      }
    }
  }
}

int main() {
  ip = "127.0.0.1";
  port = 10000;
  ddg::IOManager iom(2);
  iom.schedule(run);
  return 0;
}
