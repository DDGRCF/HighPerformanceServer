#ifndef DDG_CONFIG_H_
#define DDG_CONFIG_H_

#include <ddg/log.h>
#include <yaml-cpp/yaml.h>
#include <boost/lexical_cast.hpp>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include "utils.h"

namespace ddg {

class ConfigVarBase {
 public:
  typedef std::shared_ptr<ConfigVarBase> ptr;
  ConfigVarBase(const std::string& name, const std::string& description = "");

  virtual ~ConfigVarBase() {}

  virtual std::string toString() = 0;

  virtual bool fromString(const std::string& val) = 0;

  const std::string& getName() const { return m_name; }

  const std::string& getDescription() const { return m_description; }

  virtual std::string getTypeName() const = 0;

 protected:
  std::string m_name;
  std::string m_description;
};

/**
 * @brief 将类型转化为(F类型， T目标类型)
*/
template <class F, class T>
class LexicalCast {
 public:
  T operator()(const F& v) {
    return boost::lexical_cast<T>(v);
  }  // 转化出错将报错
};

template <class T>
class LexicalCast<std::string, std::vector<T>> {
 public:
  std::vector<T> operator()(const std::string& v) {
    YAML::Node node = YAML::Load(v);
    typename std::vector<T> vec;
    std::stringstream ss;
    for (size_t i = 0; i < node.size(); i++) {
      ss.str("");  // 重置内容
      ss.clear();  // 清空标志位
      ss << node[i];
      vec.push_back(LexicalCast<std::string, T>()(ss.str()));
    }
    return vec;
  }
};

template <class T>
class LexicalCast<std::vector<T>, std::string> {
 public:
  std::string operator()(const std::vector<T>& v) {
    YAML::Node node(YAML::NodeType::Sequence);
    for (auto& i : v) {
      node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
  }
};

template <class T>
class LexicalCast<std::string, std::list<T>> {
 public:
  std::list<T> operator()(const std::string& v) {
    YAML::Node node = YAML::Load(v);
    typename std::list<T> lis;
    std::stringstream ss;
    for (size_t i = 0; i < node.size(); i++) {
      ss.str("");
      ss.clear();
      ss << node[i];
      lis.push_back(LexicalCast<std::string, T>()(ss.str()));
    }
    return lis;
  }
};

template <class T>
class LexicalCast<std::list<T>, std::string> {
 public:
  std::string operator()(const std::list<T>& v) {
    YAML::Node node(YAML::NodeType::Sequence);
    for (auto& i : v) {
      node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
  }
};

template <class T>
class LexicalCast<std::string, std::set<T>> {
 public:
  std::set<T> operator()(const std::string& v) {
    YAML::Node node = YAML::Load(v);
    typename std::set<T> st;
    std::stringstream ss;
    for (size_t i = 0; i < node.size(); i++) {
      ss.str("");
      ss.clear();
      ss << node[i];
      st.insert(LexicalCast<std::string, T>()(ss.str()));
    }
    return st;
  }
};

template <class T>
class LexicalCast<std::set<T>, std::string> {
 public:
  std::string operator()(const std::set<T>& v) {
    YAML::Node node(YAML::NodeType::Sequence);
    for (auto& i : v) {
      node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
  }
};

template <class T>
class LexicalCast<std::string, std::unordered_set<T>> {
 public:
  std::unordered_set<T> operator()(const std::string& v) {
    YAML::Node node = YAML::Load(v);
    typename std::unordered_set<T> st;
    std::stringstream ss;
    for (size_t i = 0; i < node.size(); i++) {
      ss.str("");
      ss.clear();
      ss << node[i];
      st.insert(LexicalCast<std::string, T>()(ss.str()));
    }
    return st;
  }
};

template <class T>
class LexicalCast<std::unordered_set<T>, std::string> {
 public:
  std::string operator()(const std::unordered_set<T>& v) {
    YAML::Node node(YAML::NodeType::Sequence);
    for (auto& i : v) {
      node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
  }
};

template <class T>
class LexicalCast<std::string, std::map<std::string, T>> {
 public:
  std::map<std::string, T> operator()(const std::string& v) {
    YAML::Node node = YAML::Load(v);
    typename std::map<std::string, T> mp;
    std::stringstream ss;
    for (auto it = node.begin(); it != node.end(); it++) {
      ss.str("");
      ss.clear();
      ss << it->second;
      mp.insert(
          std::make_pair(it->first, LexicalCast<std::string, T>()(it->second)));
    }
    return mp;
  }
};

template <class T>
class LexicalCast<std::map<std::string, T>, std::string> {
 public:
  std::string operator()(const std::map<std::string, T>& v) {
    YAML::Node node(YAML::NodeType::Sequence);
    for (auto& i : v) {
      node.force_insert(i.first,
                        YAML::Load(LexicalCast<T, std::string>()(i.second)));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
  }
};

template <class T>
class LexicalCast<std::string, std::unordered_map<std::string, T>> {
 public:
  std::unordered_map<std::string, T> operator()(const std::string& v) {
    YAML::Node node = YAML::Load(v);
    typename std::unordered_map<std::string, T> mp;
    std::stringstream ss;
    for (auto it = node.begin(); it != node.end(); it++) {
      ss.str("");
      ss.clear();
      ss << it->second;
      mp.insert(std::make_pair(it->first.Scalar(),
                               LexicalCast<std::string, T>()(ss.str())));
    }
    return mp;
  }
};

template <class T>
class LexicalCast<std::unordered_map<std::string, T>, std::string> {
 public:
  std::string operator()(const std::unordered_map<std::string, T>& v) {
    YAML::Node node(YAML::NodeType::Sequence);
    for (auto& i : v) {
      node.force_insert(i.first,
                        YAML::Load(LexicalCast<T, std::string>()(i.second)));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
  }
};

template <class T, class FromStr = LexicalCast<std::string, T>,
          class ToStr = LexicalCast<T, std::string>>
class ConfigVar : public ConfigVarBase {
 public:
  typedef std::shared_ptr<ConfigVar> ptr;

  // typedef std::function<void(const T& old_value, const T& new_value)>
  // on_change_cb;

  ConfigVar(const std::string& name, const T& default_value,
            const std::string& description = "")
      : ConfigVarBase(name, description), m_val(default_value) {}

  std::string toString() override {
    try {
      return ToStr()(m_val);
    } catch (std::exception& e) {
      DDG_LOG_ERROR(DDG_LOG_ROOT())
          << "ConfigVar::fromString exception " << e.what()
          << " convert: " << TypeToName<T>() << " to string"
          << " name = " << m_name;
    }
    return "";
  }

  bool fromString(const std::string& val) override {
    try {
      setValue(FromStr()(val));
      return true;
    } catch (std::exception& e) {
      DDG_LOG_ERROR(DDG_LOG_ROOT())
          << "ConfigVar::fromString exception " << e.what()
          << " convert string to " << TypeToName<T>() << " name = " << m_name
          << " - " << val;
    }
    return false;
  }

  void setValue(const T& v) {
    if (m_val == v) {
      return;
    }
    m_val = v;
  }

  T getValue() { return m_val; }

  std::string getTypeName() const override { return TypeToName<T>(); }

 private:
  T m_val;
};

// Config
class Config {
 public:
  typedef std::unordered_map<std::string, ConfigVarBase::ptr> ConfigVarMap;

  template <class T>
  using ConfigVarPtr = typename ConfigVar<T>::ptr;

  static const std::string kValidSet;

  template <class T>
  static ConfigVarPtr<T> Lookup(const std::string& name, const T& default_value,
                                const std::string& description = "") {

    auto it = GetDatas().find(name);
    if (it != GetDatas().end()) {
      auto tmp = std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
      if (tmp) {
        DDG_LOG_INFO(DDG_LOG_ROOT()) << "Lookup name: " << name << " exists";
        return tmp;
      } else {
        DDG_LOG_ERROR(DDG_LOG_ROOT())
            << "Lookup name = " << name << " exists but type not "
            << TypeToName<T>() << " real_type = " << it->second->getTypeName()
            << " " << it->second->toString();
      }
    }

    if (name.find_first_not_of(kValidSet) != std::string::npos) {
      DDG_LOG_ERROR(DDG_LOG_ROOT()) << "Config invalid name: " << name;
      throw std::invalid_argument(name);
    }

    ConfigVarPtr<T> elem =
        std::make_shared<ConfigVar<T>>(name, default_value, description);
    GetDatas().insert(std::make_pair(name, elem));

    return elem;
  }

  template <class T>
  static ConfigVarPtr<T> Lookup(const std::string& name) {
    auto it = GetDatas().find(name);
    if (it == GetDatas().end()) {
      return nullptr;
    }
    return std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
  }

  static void LoadFromYaml(const YAML::Node& root);

  static void ListAllMember(
      const std::string& prefix, const YAML::Node& node,
      std::list<std::pair<std::string, const YAML::Node>>& output);

  static ConfigVarBase::ptr LookupBase(const std::string& name);

 private:
  // static ConfigVarMap s_datas;  // 程序开始就初始化
  static ConfigVarMap&
  GetDatas() {  // 在第一次调用的时候才初始化，避免慢启动问题，控制创建时机，避免不必要的开销
    static ConfigVarMap s_datas;
    return s_datas;
  }
};

}  // namespace ddg

#endif
