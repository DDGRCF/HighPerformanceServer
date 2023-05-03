#include <pthread.h>
#include <time.h>
#include <iostream>
#include <memory>
#include "ddg/log.h"
#include "ddg/utils.h"

int main(int argc, char** argv) {
  auto logger = DDG_LOG_NAME("root");

  DDG_LOG_INFO(logger) << "rcf is test";
  return 0;
}
