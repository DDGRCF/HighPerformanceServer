#include <assert.h>
#include "ddg/log.h"
#include "ddg/utils.h"

auto g_logger = DDG_LOG_ROOT();

void test_assert() {
  DDG_LOG_INFO(g_logger) << ddg::BacktraceToString();
}

int main() {
  test_assert();
  return 0;
}
