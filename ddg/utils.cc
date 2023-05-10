#include "utils.h"

#include <pthread.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include <thread>

namespace ddg {

uint64_t getThreadId() {
  return syscall(SYS_gettid);
}

uint64_t getFiberId() {
  return 0;  // TODO:
}

}  // namespace ddg
