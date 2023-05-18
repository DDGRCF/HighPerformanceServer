#include <vector>
#include "ddg/fiber.h"
#include "ddg/log.h"
#include "ddg/thread.h"

static ddg::Logger::ptr g_logger = DDG_LOG_ROOT();

void run_in_fiber() {
  DDG_LOG_INFO(g_logger) << "run_in_fiber begin";
  ddg::Fiber::YieldToHold();
  DDG_LOG_INFO(g_logger) << "run_in_fiber_end";
  ddg::Fiber::YieldToHold();
}

int main(int argc, char** argv) {

  std::vector<ddg::Thread::ptr> threads;
  for (int i = 0; i < 5; i++) {
    threads.push_back(
        std::make_shared<ddg::Thread>("test_" + std::to_string(i), []() {
          auto main_fiber = ddg::Fiber::GetThis();
          DDG_LOG_DEBUG(g_logger)
              << "main fiber count: " << main_fiber.use_count();
          ddg::Fiber::ptr fiber = std::make_shared<ddg::Fiber>(run_in_fiber);
          fiber->swapIn();
          DDG_LOG_INFO(g_logger) << "main after swapIn";
          fiber->swapIn();
          DDG_LOG_INFO(g_logger) << "main after end";
          fiber->swapIn();
        }));
  }

  for (auto& thread : threads) {
    thread->join();
  }
  return 0;
}
