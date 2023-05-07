#ifndef DDG_CONFIG_H_
#define DDG_CONFIG_H_

#include <yaml-cpp/yaml.h>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include "ddg/lexicalcast.h"
#include "ddg/log.h"
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

template <class T, class FromStr = LexicalCast<std::string, T>,
          class ToStr = LexicalCast<T, std::string>>
class ConfigVar : public ConfigVarBase {
 public:
  typedef std::shared_ptr<ConfigVar> ptr;

  typedef std::function<void(const T& old_value, const T& new_value)>
      on_change_cb;  // 回调函数监视事件

  template <class K>
  friend std::ostream& operator<<(std::ostream& os, ConfigVar<K> var);

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
    for (auto& i : m_cbs) {
      i.second(m_val, v);
    }
    m_val = v;
  }

  T getValue() { return m_val; }

  std::string getTypeName() const override { return TypeToName<T>(); }

  uint64_t addListener(on_change_cb cb) {
    for (uint64_t i = 0; i <= s_func_id; i++) {
      if (m_cbs.find(i) == m_cbs.end()) {
        m_cbs.insert(std::make_pair(i, cb));
        return i;
      }
    }

    m_cbs.insert(std::make_pair(++s_func_id, cb));
    return s_func_id;
  }

  void addListener(uint64_t key, on_change_cb cb) {
    if (key > s_func_id) {
      s_func_id = key;
    }
    m_cbs.insert(std::make_pair(key, cb));
  }

  void dealListener(uint64_t key) { m_cbs.earse(key); }

  void clearListener() { m_cbs.clear(); }

  on_change_cb getListener(uint64_t key) {
    auto it = m_cbs.find(key);
    return it == m_cbs.end() ? nullptr : it->second;
  }

 private:
  T m_val;

  uint64_t s_func_id = 0;
  std::unordered_map<uint64_t, on_change_cb>
      m_cbs;  // 加map是因为回调函数没有等于
};

template <class T>
std::ostream& operator<<(std::ostream& os, ConfigVar<T> var) {
  os << var.toString();
  return os;
}

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
  // 在第一次调用的时候才初始化，避免慢启动问题，控制创建时机，避免不必要的开销
  // 最主要的原因是保证static初始化元素之间没有初始化顺序，通过这样能够明确调用顺序
  static ConfigVarMap& GetDatas() {
    static ConfigVarMap s_datas;
    return s_datas;
  }
};

}  // namespace ddg

#endif
