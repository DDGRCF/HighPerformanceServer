#include "utils.h"

#include <pthread.h>
#include <thread>

namespace ddg {

uint64_t getThreadId() {
  return pthread_self();
}

uint64_t getFiberId() {
  return 0;  // TODO:
}

}  // namespace ddg
