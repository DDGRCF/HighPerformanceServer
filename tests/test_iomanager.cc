#include "ddg/iomanager.h"
#include "ddg/log.h"

static ddg::Logger::ptr g_logger = DDG_LOG_ROOT();

int main() {
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
  return 0;
}
