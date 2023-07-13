#include <arpa/inet.h>
#include <fcntl.h>

#include <sys/socket.h>
#include <unistd.h>
#include "ddg/hook.h"
#include "ddg/iomanager.h"
#include "ddg/log.h"
#include "ddg/macro.h"

int sockfd;
static ddg::Logger::ptr g_logger = DDG_LOG_ROOT();

void watch_io_read();

void test_iomanager1() {
  ddg::IOManager iom(1, "", true);  // 不知道为什么暂时不可以用
  auto test_func1 = []() {
    static int i = 0;
    DDG_LOG_DEBUG(g_logger) << "id [" << i << "] test_iomanager1 start ...";
    sleep(2);
    DDG_LOG_DEBUG(g_logger) << "id [" << i << "] test_iomanager1 end ...";
    i++;
  };

  iom.schedule(test_func1);
  iom.start();
  iom.stop();
}

void test_iomanager2() {
  const int Port = 80;
  const char* Addr = "127.0.0.1";

  ddg::IOManager iom(1, "iomanager2", true);
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
    ddg::IOManager::GetThis()->addEvent(sfd, ddg::IOManager::Event::READ, []() {
      DDG_LOG_DEBUG(g_logger) << "test_iomanager2 read callback";
    });

    // 缓冲区为空可以写，触发写事件，在写事件里面触发读事件
    ddg::IOManager::GetThis()->addEvent(
        sfd, ddg::IOManager::Event::WRITE, [sfd]() {
          DDG_LOG_DEBUG(g_logger) << "test_iomanager2 write callback";
          ddg::IOManager::GetThis()->cancelEvent(sfd,
                                                 ddg::IOManager::Event::READ);
          close(sfd);
        });
  } else {
    DDG_LOG_DEBUG(g_logger) << "else " << errno << " " << strerror(errno);
  }
  iom.stop();
}

// 写事件回调，只执行一次，用于判断非阻塞套接字connect成功
void do_io_write() {
  DDG_LOG_INFO(g_logger) << "test_iomanager3 write callback";
  int so_err;
  socklen_t len = sizeof(so_err);
  getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &so_err, &len);
  if (so_err) {
    DDG_LOG_INFO(g_logger) << "test_iomanager3 connect fail";
    return;
  }
  DDG_LOG_INFO(g_logger) << "test_iomanager3 connect success";
  const char* buf = "123456789\n";
  int ret = write(sockfd, buf, strlen(buf));
  if (ret < 0) {
    DDG_LOG_ERROR(g_logger)
        << "err, errno=" << errno << ", errstr=" << strerror(errno);
    close(sockfd);
  } else {
    DDG_LOG_DEBUG(g_logger) << "write " << ret;
  }
}

// 读事件回调，每次读取之后如果套接字未关闭，需要重新添加
void do_io_read() {
  DDG_LOG_INFO(g_logger) << "test_iomanager3 read callback";
  char buf[1024] = {0};
  int readlen = 0;
  readlen = read(sockfd, buf, sizeof(buf));
  if (readlen > 0) {
    buf[readlen] = '\0';
    DDG_LOG_INFO(g_logger) << "test_iomanager3 read " << readlen
                           << " bytes, read: " << buf;
  } else if (readlen == 0) {
    DDG_LOG_INFO(g_logger) << "test_iomanager3 peer closed";
    close(sockfd);
    return;
  } else {
    DDG_LOG_ERROR(g_logger)
        << "err, errno=" << errno << ", errstr=" << strerror(errno);
    close(sockfd);
    return;
  }
  // read之后重新添加读事件回调，这里不能直接调用addEvent，因为在当前位置fd的读事件上下文还是有效的，直接调用addEvent相当于重复添加相同事件
  ddg::IOManager::GetThis()->schedule(watch_io_read);
}

void watch_io_read() {
  DDG_LOG_INFO(g_logger) << "test_iomanager3 watch_io_read";
  ddg::IOManager::GetThis()->addEvent(sockfd, ddg::IOManager::Event::READ,
                                      do_io_read);
}

void test_io() {
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  DDG_ASSERT(sockfd > 0);
  // fcntl(sockfd, F_SETFL, O_NONBLOCK);

  sockaddr_in servaddr;
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(80);
  inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr.s_addr);

  int rt = connect(sockfd, (const sockaddr*)&servaddr, sizeof(servaddr));
  if (!rt || (rt != 0 && errno == EINPROGRESS)) {
    if (errno == EINPROGRESS) {
      DDG_LOG_DEBUG(g_logger) << "test_iomanager3 connect in EINPROGRESS";
    }
    DDG_ASSERT(ddg::IOManager::GetThis() != nullptr);
    DDG_ASSERT(ddg::IOManager::GetThis()->addEvent(
        sockfd, ddg::IOManager::Event::WRITE, do_io_write));
    // 注册读事件回调，注意事件是一次性的
    DDG_ASSERT(ddg::IOManager::GetThis()->addEvent(
        sockfd, ddg::IOManager::Event::READ, do_io_read));
  } else {
    DDG_LOG_ERROR(g_logger) << "test_iomanager3 else, errno:" << errno
                            << ", errstr:" << strerror(errno);
  }
}

void test_iomanager3() {
  ddg::IOManager iom(2, "test", true);
  iom.schedule(test_io);
  iom.start();
  iom.stop();
}

int main() {
  // 循环测试，看看有没有偶然性错误
  for (;;) {
    DDG_LOG_DEBUG(g_logger) << "test";
    ddg::Thread::ptr t1 =
        std::make_shared<ddg::Thread>("thread 1", test_iomanager1);

    ddg::Thread::ptr t2 =
        std::make_shared<ddg::Thread>("thread 2", test_iomanager2);

    ddg::Thread::ptr t3 =
        std::make_shared<ddg::Thread>("thread 3", test_iomanager3);

    t1->join();
    t2->join();
    t3->join();
  }
  return 0;
}
