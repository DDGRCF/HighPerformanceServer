#include <ddg/config.h>
#include <ddg/log.h>
#include <yaml-cpp/yaml.h>
#include <iostream>
#include <iterator>
#include <unordered_map>

#define DDG_LOG_VECTOR_DEBUG(logger, vec) \
  {                                       \
    std::string res = "";                 \
    for (auto& i : vec) {                 \
      if (res.empty()) {                  \
        res = std::to_string(i);          \
      } else {                            \
        res += ", " + std::to_string(i);  \
      }                                   \
    }                                     \
    DDG_LOG_DEBUG(logger) << res;         \
  }

#define DDG_LOG_MAP_DEBUG(logger, map)                            \
  {                                                               \
    std::string res = "";                                         \
    for (auto& i : map) {                                         \
      if (res.empty()) {                                          \
        res = i.first + ", " + std::to_string(i.second);          \
      } else {                                                    \
        res += " | " + i.first + ", " + std::to_string(i.second); \
      }                                                           \
    }                                                             \
    DDG_LOG_DEBUG(logger) << res;                                 \
  }

int main() {
  auto g_logger = DDG_LOG_ROOT();

  DDG_LOG_DEBUG(g_logger) << "Test for Config";
  {

    ddg::ConfigVar<int>::ptr g_int_value_config = ddg::Config::Lookup<int>(
        "system.port", static_cast<int>(8080), "system port");

    ddg::ConfigVar<float>::ptr g_int_valuex_config = ddg::Config::Lookup<float>(
        "system.port", static_cast<float>(8080), "system port");

    ddg::ConfigVar<float>::ptr g_float_value_config =
        ddg::Config::Lookup<float>("system.value", 2.32f, "system value");

    ddg::ConfigVar<std::vector<int>>::ptr g_vector_value_config =
        ddg::Config::Lookup<std::vector<int>>(
            "system.vector", std::vector<int>{1, 2}, "system vector");
    DDG_LOG_DEBUG(g_logger) << g_vector_value_config->toString();
  }

  DDG_LOG_DEBUG(g_logger) << "Test for LexicalCast";
  {
    std::vector<int> in1{1, 2, 3, 4, 5};
    std::string in1_str =
        ddg::LexicalCast<std::vector<int>, std::string>()(in1);

    DDG_LOG_INFO(g_logger) << in1_str;

    std::vector<int> out1 =
        ddg::LexicalCast<std::string, std::vector<int>>()(in1_str);
    DDG_LOG_VECTOR_DEBUG(g_logger, out1);

    std::unordered_map<std::string, int> in2{
        {"rcf", 18}, {"rrr", 20}, {"ccc", 21}};
    std::string in2_str =
        ddg::LexicalCast<std::unordered_map<std::string, int>, std::string>()(
            in2);
    DDG_LOG_INFO(g_logger) << in2_str;

    std::unordered_map<std::string, int> out2 =
        ddg::LexicalCast<std::string, std::unordered_map<std::string, int>>()(
            in2_str);

    DDG_LOG_MAP_DEBUG(g_logger, out2);
  }

  return 0;
}
