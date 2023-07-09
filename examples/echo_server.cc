#include "ddg/address.h"
#include "ddg/bytearray.h"
#include "ddg/iomanager.h"
#include "ddg/log.h"
#include "ddg/socket.h"
#include "ddg/tcp_server.h"

static ddg::Logger::ptr g_logger = DDG_LOG_ROOT();

class EchoServer : public ddg::TcpServer {
 public:
  EchoServer(int type);

  void handleClient(ddg::Socket::ptr client) override;

 private:
  int m_type = 0;
};

EchoServer::EchoServer(int type) : m_type(type) {}

void EchoServer::handleClient(ddg::Socket::ptr client) {
  DDG_LOG_INFO(g_logger) << "handleClient" << *client;
  ddg::ByteArray::ptr ba(new ddg::ByteArray);
  int ret;
  while (true) {
    ba->clear();
    std::vector<iovec> iovs;
    ba->getWriteBuffers(iovs, 1024);
    ret = client->recv(&iovs[0], iovs.size());
    if (ret == 0) {
      DDG_LOG_DEBUG(g_logger) << "client close : " << *client;
      break;
    } else if (ret < 0) {
      DDG_LOG_DEBUG(g_logger)
          << "client error ret = " << ret << " errno = " << errno
          << " errstr = " << strerror(errno);
      break;
    }

    ba->setPosition(ba->getPosition() + ret);
    ba->setPosition(0);

    if (m_type == 1) {
      std::cout << ba->toString();
    } else {
      std::cout << ba->toHexString();
    }
    std::cout << std::endl;
  }
}

const int type = 1;

void run() {
  DDG_LOG_DEBUG(g_logger) << "server type = " << type;
  EchoServer::ptr es = std::make_shared<EchoServer>(type);
  auto addr = ddg::Address::LookupAny("0.0.0.0:8020");
  while (!es->bind(addr)) {
    sleep(2);
  }
  es->start();
}

int main() {
  ddg::IOManager iom(2);
  iom.start();
  iom.schedule(run);
  iom.stop();
  return 0;
}
