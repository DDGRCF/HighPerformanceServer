#include "ddg/config.h"
#include <algorithm>

namespace ddg {

// ConfigVarBase
ConfigVarBase::ConfigVarBase(const std::string& name,
                             const std::string& description)
    : m_name(name), m_description(description) {
  std::transform(name.begin(), name.end(), m_name.begin(), ::tolower);
}

ConfigVarBase::~ConfigVarBase() {}

const std::string& ConfigVarBase::getName() const {
  RWMutexType::ReadLock lock(m_rwmutex);
  return m_name;
}

const std::string& ConfigVarBase::getDescription() const {
  RWMutexType::ReadLock lock(m_rwmutex);
  return m_description;
}

ConfigVarBase::ptr Config::LookupBase(const std::string& name) {
  RWMutexType::ReadLock lock(GetMutex());
  auto it = GetDatas().find(name);
  return it == GetDatas().end() ? nullptr : it->second;
}

const std::string& Config::kValidSet() {
  const static std::string val = "abcdefghijklmnopqrstuvwxyz._0123456789";
  return val;
}

void Config::ListAllMember(
    const std::string& prefix, const YAML::Node& node,
    std::list<std::pair<std::string, const YAML::Node>>& output) {
  RWMutexType::ReadLock lock(GetMutex());
  if (prefix.find_first_not_of(kValidSet()) != std::string::npos) {
    DDG_LOG_ERROR(DDG_LOG_ROOT())
        << "Config invalid name: " << prefix << " : " << node;
    return;
  }

  output.push_back(std::make_pair(prefix, node));  // 递归获取层级结构
  if (node.IsMap()) {
    for (auto it = node.begin(); it != node.end(); it++) {
      ListAllMember(prefix.empty() ? it->first.Scalar()
                                   : prefix + "." + it->first.Scalar(),
                    it->second, output);
    }
  }
}

void Config::Visit(VisitCallback cb) {
  RWMutexType::ReadLock lock(Config::GetMutex());
  for (auto it = GetDatas().begin(); it != GetDatas().end(); it++) {
    cb(it->second);
  }
}

void Config::LoadFromYaml(const YAML::Node& root) {
  std::list<std::pair<std::string, const YAML::Node>> all_nodes;
  ListAllMember("", root, all_nodes);

  RWMutexType::ReadLock lock(GetMutex());
  for (auto& i : all_nodes) {
    std::string key = i.first;
    if (key.empty()) {
      continue;
    }

    std::transform(key.begin(), key.end(), key.begin(), ::tolower);
    ConfigVarBase::ptr var = LookupBase(key);

    if (var) {
      if (i.second.IsScalar()) {
        var->fromString(i.second.Scalar());
      } else {
        std::stringstream ss;
        ss << i.second;  // 这里重载了内部的流操作符
        var->fromString(ss.str());
      }
    }
  }
}

}  // namespace ddg
