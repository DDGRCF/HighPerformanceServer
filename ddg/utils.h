#ifndef DDG_UTILS_H_
#define DDG_UTILS_H_

#include <cxxabi.h>
#include <stdint.h>
#include <string>
#include <vector>

#include "ddg/noncopyable.h"
#include "ddg/utils/json_util.h"
#include "ddg/utils/memory_util.h"
#include "ddg/utils/string_util.h"
#include "ddg/utils/type_util.h"

#include <jsoncpp/json/json.h>
#include <yaml-cpp/yaml.h>

namespace ddg {

pid_t GetThreadId();

pid_t GetFiberId();
time_t GetCurrentMicroSecond();

time_t GetCurrentMilliSecond();

std::string BacktraceToString(int size = 64, int skip = 1,
                              const std::string& prefix = "");

bool YamlToJson(const YAML::Node& ynode, Json::Value& jnode);

bool JsonToYaml(const Json::Value& jnode, YAML::Node& ynode);

template <typename T>
const char* TypeToName() {
  static const char* s_name =
      abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
  return s_name;
}

}  // namespace ddg

#endif
