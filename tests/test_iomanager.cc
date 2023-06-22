#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>
#include "ddg/iomanager.h"
#include "ddg/log.h"
#include "ddg/macro.h"

static ddg::Logger::ptr g_logger = DDG_LOG_ROOT();

void test_iomanager1() {
  ddg::IOManager iom(3, true, "iomanager");
  auto test_func1 = []() {
    DDG_LOG_DEBUG(g_logger) << "in test_func1 start ...";
    sleep(2);
    ddg::Fiber::Yield();
    DDG_LOG_DEBUG(g_logger) << "in test_func1 end ...";
  };

  iom.start();
  iom.schedule(test_func1);
  iom.stop();
}

void test_iomanager2() {
  const int Port = 80;
  const char* Addr = "127.0.0.1";

  ddg::IOManager iom(4, true, "iomanager");
  iom.start();

  struct sockaddr_in addr;

  addr.sin_port = htons(Port);
  int ret = inet_pton(AF_INET, Addr, &addr.sin_addr.s_addr);
  DDG_ASSERT(ret == 1);
  addr.sin_family = AF_INET;

  char ip[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip));
  DDG_LOG_DEBUG(g_logger) << ip;

  int sfd = socket(AF_INET, SOCK_STREAM, 0);
  DDG_ASSERT(sfd >= 0);

  fcntl(sfd, F_SETFL, O_NONBLOCK);

  ret = connect(sfd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
  if (ret == 0) {
    DDG_LOG_DEBUG(g_logger) << "connect success";
  } else if (
      errno ==
      EINPROGRESS) {  // 使用非阻塞方式进行链接的时候会发生，说明链接还在继续
    DDG_LOG_DEBUG(g_logger)
        << "add event errno = " << errno << " " << strerror(errno);
    ddg::IOManager::GetThis()->addEvent(sfd, ddg::IOManager::READ, []() {
      DDG_LOG_DEBUG(g_logger) << "read callback";
    });

    ddg::IOManager::GetThis()->addEvent(sfd, ddg::IOManager::WRITE, [sfd]() {
      DDG_LOG_DEBUG(g_logger) << "write callback";
      ddg::IOManager::GetThis()->cancelEvent(sfd, ddg::IOManager::READ);
      close(sfd);
    });
  } else {
    DDG_LOG_DEBUG(g_logger) << "else " << errno << " " << strerror(errno);
  }
  iom.stop();
}

void test_timer() {
  ddg::IOManager iom(2, true, "iomanager");

  iom.start();
  ddg::Timer::ptr s_timer;
  s_timer = iom.addTimer(
      1000,
      [s_timer]() {
        static int i = 0;
        DDG_LOG_DEBUG(g_logger) << "hello timer i = " << i;
        if (++i == 3) {
          s_timer->reset(2000, true);
        }
      },
      true);
  iom.stop();
}

int main() {
  test_iomanager2();
  // test_timer();
  return 0;
}
