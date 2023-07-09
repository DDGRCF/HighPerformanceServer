#include "ddg/hook.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "ddg/iomanager.h"
#include "ddg/log.h"

static ddg::Logger::ptr g_logger = DDG_LOG_ROOT();

void test() {
  ddg::IOManager iom(2);
  iom.start();

  auto test_sleep = [&iom]() {
    iom.schedule([]() {
      sleep(2);
      DDG_LOG_DEBUG(g_logger) << "sleep 2";
    });
    iom.schedule([]() {
      sleep(3);
      DDG_LOG_DEBUG(g_logger) << "sleep 3";
    });

    DDG_LOG_DEBUG(g_logger) << "test_sleep";
  };

  test_sleep();

  // TODO:
  auto test_socket = [&iom]() {
    auto func = []() {
      int sock = socket(AF_INET, SOCK_STREAM, 0);

      sockaddr_in addr;
      memset(&addr, 0, sizeof(addr));
      addr.sin_family = AF_INET;
      addr.sin_port = htons(80);
      int ret = inet_pton(AF_INET, "110.242.68.4", &addr.sin_addr.s_addr);
      if (!ret) {
        DDG_LOG_DEBUG(g_logger)
            << "inet_pton ret = " << ret << " errno = " << errno
            << " strerror = " << strerror(errno);
        return;
      }

      DDG_LOG_DEBUG(g_logger) << "begin connect";
      ret = connect(sock, (const sockaddr*)&addr, sizeof(addr));
      if (ret) {
        DDG_LOG_DEBUG(g_logger) << "connect ret=" << ret << " errno=" << errno
                                << " errstr = " << strerror(errno);
        return;
      }

      const char data[] = "GET / HTTP/1.0\r\n\r\n";
      ret = send(sock, data, sizeof(data), 0);

      if (ret <= 0) {
        DDG_LOG_DEBUG(g_logger) << "send ret=" << ret << " errno=" << errno
                                << " errstr = " << strerror(errno);
        return;
      }

      std::string buff;
      buff.resize(4096);

      ret = recv(sock, &buff[0], buff.size(), 0);
      DDG_LOG_DEBUG(g_logger) << "recv ret=" << ret << " errno=" << errno
                              << " errstr = " << strerror(errno);

      if (ret <= 0) {
        return;
      }

      buff.resize(ret);
      DDG_LOG_DEBUG(g_logger) << buff;
    };
    iom.schedule(func);
  };

  test_socket();
}

int main() {
  test();
  return 0;
}
