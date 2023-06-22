#include "ddg/fiber.h"
#include "ddg/log.h"
#include "ddg/scheduler.h"
#include "ddg/thread.h"

static ddg::Logger::ptr g_logger = DDG_LOG_ROOT();

void test_call() {
  static int s_count = 5;
  DDG_LOG_DEBUG(g_logger) << "test_call start "
                          << "id " << s_count;
  ddg::Fiber::Yield();
  sleep(1);
  DDG_LOG_DEBUG(g_logger) << "test_call end "
                          << "id " << s_count;
  if (s_count > 0) {
    ddg::Scheduler::GetThis()->schedule(&test_call, ddg::GetThreadId());
    s_count--;
  }
}

int main() {
  ddg::Scheduler scheduler(2, true, "test");
  for (int i = 0; i < 3; i++) {
    scheduler.schedule([i]() {
      DDG_LOG_DEBUG(g_logger) << "id: " << i << " test_call start";
      ddg::Fiber::Yield();
      DDG_LOG_DEBUG(g_logger) << "id " << i << " test_call end";
    });
  }

  scheduler.start();
  scheduler.schedule(test_call);
  scheduler.stop();
  DDG_LOG_DEBUG(g_logger) << "test main end";
  return 0;
}
