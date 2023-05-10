#include <yaml-cpp/yaml.h>
#include <iostream>
#include <iterator>
#include <unordered_map>
#include "ddg/config.h"
#include "ddg/lexicalcast.h"
#include "ddg/log.h"

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
  auto system_logger = DDG_LOG_NAME("system");

  DDG_LOG_DEBUG(system_logger) << "sytem logger test";

  auto root_node = YAML::LoadFile("tests/confs/log.yaml");

  DDG_LOG_DEBUG(g_logger) << std::boolalpha << root_node["system"].IsDefined();
  DDG_LOG_DEBUG(g_logger) << std::boolalpha
                          << root_node["system"]["port"].IsDefined();
  DDG_LOG_DEBUG(g_logger) << root_node;

  DDG_LOG_DEBUG(g_logger) << "Test for Config";
  {

    ddg::ConfigVar<int>::ptr g_int_value_config = ddg::Config::Lookup<int>(
        "system.port", static_cast<int>(8080), "system port");

    g_int_value_config->addListener(1, [&g_logger](const int& o, const int& n) {
      DDG_LOG_DEBUG(g_logger)
          << "change old value from " << o << " to new " << n;
    });
    //
    g_int_value_config->setValue(8889);
    ddg::ConfigVar<float>::ptr g_int_valuex_config = ddg::Config::Lookup<float>(
        "system.port", static_cast<float>(8080), "system port");
    //
    ddg::ConfigVar<float>::ptr g_float_value_config =
        ddg::Config::Lookup<float>("system.value", 2.32f, "system value");

    ddg::ConfigVar<std::vector<int>>::ptr g_vector_value_config =
        ddg::Config::Lookup<std::vector<int>>(
            "system.vector", std::vector<int>{1, 2}, "system vector");

    DDG_LOG_DEBUG(g_logger) << g_vector_value_config->toString();

    ddg::Config::LoadFromYaml(root_node);
    DDG_LOG_DEBUG(g_logger) << g_int_value_config->toString();

    ddg::ConfigVar<std::set<ddg::LogDefine>>::ptr g_logdefine_set_value_config =
        ddg::Config::Lookup<std::set<ddg::LogDefine>>("logs");
    std::cout << "===========================Root" << std::endl;
    DDG_LOG_INFO(g_logger) << *g_logdefine_set_value_config;
    std::cout << "===========================System" << std::endl;
    DDG_LOG_INFO(system_logger) << *g_logdefine_set_value_config;
  }

  DDG_LOG_INFO(g_logger) << "Test for LexicalCast";
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

    std::stringstream ss;
    for (auto it = root_node["logs"].begin(); it != root_node["logs"].end();
         it++) {
      ss.str("");
      ss.clear();
      ss << *it;
      std::cout << "==============" << std::endl;
      ddg::LogDefine in3 =
          ddg::LexicalCast<std::string, ddg::LogDefine>()(ss.str());
      DDG_LOG_INFO(g_logger) << in3.getString();
      std::string out3_str =
          ddg::LexicalCast<ddg::LogDefine, std::string>()(in3);
      DDG_LOG_INFO(g_logger) << out3_str;
      std::cout << "==============" << std::endl;
    }
  }

  return 0;
}
