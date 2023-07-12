#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>
#include "ddg/iomanager.h"
#include "ddg/log.h"
#include "ddg/macro.h"

static ddg::Logger::ptr g_logger = DDG_LOG_ROOT();

static ddg::Timer::ptr s_timer;
static int timeout = 1000;

// 定时，如果小于10000，每次就从现在开始加上一个timeout
void timer_callback() {
  DDG_LOG_DEBUG(g_logger) << "timer callback, timeout = " << timeout;
  timeout += 1000;
  if (timeout < 10000) {
    s_timer->reset(timeout, true);
  } else {
    s_timer->cancel();
  }
}

void test_timer() {
  ddg::IOManager iom(2, "test_timer", true);
  // 循环定时器
  s_timer = iom.addTimer(1000, timer_callback, true);
  // 单次定时器
  iom.addTimer(500, [] { DDG_LOG_DEBUG(g_logger) << "500ms timeout"; });
  iom.addTimer(5000, [] { DDG_LOG_DEBUG(g_logger) << "5000ms timeout"; });
  iom.start();
}

int main() {
  test_timer();

  return 0;
}
