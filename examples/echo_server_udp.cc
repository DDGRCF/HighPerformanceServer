#include "ddg/iomanager.h"
#include "ddg/log.h"
#include "ddg/socket.h"

static ddg::Logger::ptr g_logger = DDG_LOG_ROOT();

void run() {
  ddg::IPAddress::ptr addr = ddg::Address::LookupAnyIPAddress("0.0.0.0:8050");
  ddg::Socket::ptr sock = ddg::Socket::CreateUdp(addr);
  if (sock->bind(addr)) {
    DDG_LOG_INFO(g_logger) << "udp bind : " << *addr;
  } else {
    DDG_LOG_ERROR(g_logger) << "udp bind : " << *addr << " fail";
    return;
  }
  while (true) {
    char buff[1024];
    ddg::Address::ptr from(new ddg::IPv4Address);
    int len = sock->recvFrom(buff, 1024, from);
    if (len > 0) {
      buff[len] = '\0';
      DDG_LOG_INFO(g_logger) << "recv: " << buff << " from: " << *from;
      len = sock->sendTo(buff, len, from);
      if (len < 0) {
        DDG_LOG_INFO(g_logger)
            << "send: " << buff << " to: " << *from << " error=" << len;
      }
    } else {
      DDG_LOG_DEBUG(g_logger)
          << "recv from " << *addr << " fails errno = " << errno
          << " errstr = " << strerror(errno);
    }
  }
}

int main(int argc, char** argv) {
  ddg::IOManager iom(2);
  iom.schedule(run);
  return 0;
}
