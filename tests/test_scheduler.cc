#include "ddg/fiber.h"
#include "ddg/log.h"
#include "ddg/scheduler.h"
#include "ddg/thread.h"

static ddg::Logger::ptr g_logger = DDG_LOG_ROOT();

void test_call() {
  DDG_LOG_DEBUG(g_logger) << "id: [norm] test_call start ";
  sleep(5);
  ddg::Fiber::Yield();
  DDG_LOG_DEBUG(g_logger) << "id: [norm] test_call end ";
  ddg::Scheduler scheduler(2, "test", false);
  scheduler.schedule(test_call);
}

int main() {
  ddg::Scheduler scheduler(2, "test", true);

  scheduler.schedule(test_call);
  scheduler.start();
  for (int i = 0; i < 20; i++) {
    scheduler.schedule([i]() {
      DDG_LOG_DEBUG(g_logger) << "id: [" << i << "] test_call start";
      sleep(1);
      ddg::Fiber::Yield();
      DDG_LOG_DEBUG(g_logger) << "id [" << i << "] test_call end";
    });
  }

  DDG_LOG_DEBUG(g_logger) << "test main end";
  return 0;
}
