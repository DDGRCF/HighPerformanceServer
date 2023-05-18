#include "ddg/log.h"
#include "ddg/mutex.h"
#include "ddg/thread.h"

static const size_t kNum = 50000;

int main() {
  auto g_logger = DDG_LOG_ROOT();

  for (int id = 0; id < 10; id++) {
    ddg::Thread t("test1", [&g_logger, id]() {
      for (size_t i = 0; i < kNum; i++)
        DDG_LOG_INFO(g_logger) << "thread: " << id << " " << i;
    });
  }

  sleep(10);
  return 0;
}
