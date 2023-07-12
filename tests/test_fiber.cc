#include <vector>
#include "ddg/fiber.h"
#include "ddg/log.h"
#include "ddg/thread.h"

static ddg::Logger::ptr g_logger = DDG_LOG_ROOT();

void run_in_fiber(int i) {
  DDG_LOG_INFO(g_logger) << "fiber [" << i << "] run_in_fiber begin";
  ddg::Fiber::Yield();
  DDG_LOG_INFO(g_logger) << "fiber [" << i << "] run_in_fiber medium";
  ddg::Fiber::Yield();
  DDG_LOG_INFO(g_logger) << "fiber [" << i << "] run_in_fiber end";
}

int main(int argc, char** argv) {

  std::vector<ddg::Thread::ptr> threads;
  for (int i = 0; i < 5; i++) {
    threads.push_back(
        std::make_shared<ddg::Thread>("test_" + std::to_string(i), [i] {
          auto main_fiber = ddg::Fiber::GetThis();
          DDG_LOG_DEBUG(g_logger) << "main fiber init";
          sleep(1);
          ddg::Fiber::ptr fiber =
              std::make_shared<ddg::Fiber>(std::bind(run_in_fiber, i));
          fiber->resume();
          DDG_LOG_INFO(g_logger) << "main [" << i << "] after resume firstly";
          fiber->resume();
          DDG_LOG_INFO(g_logger) << "main [" << i << "] after resume secondly";
          DDG_LOG_INFO(g_logger) << "main [" << i << "] end!";
        }));
  }

  for (auto& thread : threads) {
    thread->join();
  }
  return 0;
}
