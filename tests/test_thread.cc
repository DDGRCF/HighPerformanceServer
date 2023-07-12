#include "ddg/thread.h"

#include <vector>

#include "ddg/log.h"
#include "ddg/mutex.h"

static const size_t kNum = 50000;

int main() {
  auto g_logger = DDG_LOG_ROOT();

  std::vector<ddg::Thread::ptr> threads;
  for (int id = 0; id < 10; id++) {
    threads.push_back(std::make_shared<ddg::Thread>("test", [&g_logger, id] {
      for (size_t i = 0; i < kNum; i++)
        DDG_LOG_INFO(g_logger) << "thread: " << id << " " << i;
    }));
  }

  for (auto&& thread : threads) {
    thread->join();
  }
  DDG_LOG_DEBUG(g_logger) << "thread test term!";
  return 0;
}
