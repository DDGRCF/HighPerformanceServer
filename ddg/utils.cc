#include "ddg/utils.h"

#include <execinfo.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <algorithm>
#include <numeric>
#include <thread>

#include "ddg/fiber.h"
#include "ddg/log.h"

namespace ddg {

pid_t GetThreadId() {
  return syscall(SYS_gettid);
}

pid_t GetFiberId() {
  return ddg::Fiber::GetFiberId();
}

bool YamlToJson(const YAML::Node& ynode, Json::Value& jnode) {
  try {
    if (ynode.IsScalar()) {
      Json::Value v(ynode.Scalar());
      jnode.swapPayload(v);
      return true;
    }
    if (ynode.IsSequence()) {
      for (size_t i = 0; i < ynode.size(); ++i) {
        Json::Value v;
        if (YamlToJson(ynode[i], v)) {
          jnode.append(v);
        } else {
          return false;
        }
      }
    } else if (ynode.IsMap()) {
      for (auto it = ynode.begin(); it != ynode.end(); ++it) {
        Json::Value v;
        if (YamlToJson(it->second, v)) {
          jnode[it->first.Scalar()] = v;
        } else {
          return false;
        }
      }
    }
  } catch (...) {
    return false;
  }
  return true;
}

bool JsonToYaml(const Json::Value& jnode, YAML::Node& ynode) {
  try {
    if (jnode.isArray()) {
      for (int i = 0; i < (int)jnode.size(); ++i) {
        YAML::Node n;
        if (JsonToYaml(jnode[i], n)) {
          ynode.push_back(n);
        } else {
          return false;
        }
      }
    } else if (jnode.isObject()) {
      for (auto it = jnode.begin(); it != jnode.end(); ++it) {
        YAML::Node n;
        if (JsonToYaml(*it, n)) {
          ynode[it.name()] = n;
        } else {
          return false;
        }
      }
    } else {
      ynode = jnode.asString();
    }
  } catch (...) {
    return false;
  }
  return true;
}

void BackTrace(std::vector<std::string>& bt, int size, int skip) {
  ScopedMalloc sm(sizeof(void*) * size);
  void** array = sm.getPointer<void**>();

  size_t nptrs = ::backtrace(array, size);
  char** strings = backtrace_symbols(array, size);

  if (strings == nullptr) {
    std::system_error();
  }

  bt.resize(nptrs - skip + 1);
  for (size_t j = skip; j < nptrs; j++) {
    bt[j - skip] = strings[j];
  }

  free(strings);
}

std::string BacktraceToString(int size, int skip, const std::string& prefix) {
  std::vector<std::string> bt;
  BackTrace(bt, size, skip);
  std::stringstream ss;
  for (size_t i = 0; i < bt.size(); i++) {
    ss << prefix << bt[i] << "\n";
  }
  return ss.str();
}

time_t GetCurrentMicroSecond() {
  struct timeval tv;
  int ret = gettimeofday(&tv, nullptr);
  if (!ret) {
    throw std::system_error();
  }

  return tv.tv_sec * 1000 * 1000ul + tv.tv_usec;
}

time_t GetCurrentMilliSecond() {
  struct timeval tv;
  int ret = gettimeofday(&tv, nullptr);
  if (ret) {
    throw std::system_error();
  }
  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

}  // namespace ddg
