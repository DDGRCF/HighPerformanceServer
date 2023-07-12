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
#include "ddg/mutex.h"
#include "ddg/utils.h"

namespace ddg {

class ConfigVarBase {
 public:
  using ptr = std::shared_ptr<ConfigVarBase>;
  using RWMutexType = RWMutex;

  ConfigVarBase(const std::string& name, const std::string& description = "");

  virtual ~ConfigVarBase();

  virtual std::string toString() const = 0;

  virtual bool fromString(const std::string& val) = 0;

  const std::string& getName() const;

  const std::string& getDescription() const;

  virtual std::string getTypeName() const = 0;

 protected:
  mutable RWMutexType m_rwmutex;
  std::string m_name;
  std::string m_description;
};

template <class T, class FromStr = LexicalCast<std::string, T>,
          class ToStr = LexicalCast<T, std::string>>
class ConfigVar : public ConfigVarBase {
 public:
  using ptr = std::shared_ptr<ConfigVar>;
  using Callback = std::function<void(const T&, const T&)>;

  template <class K>
  friend std::ostream& operator<<(std::ostream& os, const ConfigVar<K>& var);

  ConfigVar(const std::string& name, const T& default_value,
            const std::string& description = "")
      : ConfigVarBase(name, description), m_val(default_value) {}

  std::string toString() const override {
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
    RWMutexType::WriteLock lock(m_rwmutex);
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

  uint64_t addListener(Callback cb) {
    RWMutexType::WriteLock lock(m_rwmutex);
    for (uint64_t i = 0; i <= s_func_id; i++) {
      if (m_cbs.find(i) == m_cbs.end()) {
        m_cbs.insert(std::make_pair(i, cb));
        return i;
      }
    }

    m_cbs.insert(std::make_pair(++s_func_id, cb));
    return s_func_id;
  }

  void addListener(uint64_t key, Callback cb) {
    RWMutexType::WriteLock lock(m_rwmutex);
    if (!funcIdGrowthValid(key)) {
      throw std::logic_error("ConfigVar::addListener invalid key");
    }

    if (key > s_func_id) {
      s_func_id = key;
    }

    m_cbs.insert(std::make_pair(key, cb));
  }

  void dealListener(uint64_t key) {
    RWMutexType::WriteLock lock(m_rwmutex);
    m_cbs.earse(key);
  }

  void clearListener() {
    RWMutexType::WriteLock lock(m_rwmutex);
    m_cbs.clear();
  }

  Callback getListener(uint64_t key) {
    RWMutexType::ReadLock lock(m_rwmutex);
    auto it = m_cbs.find(key);
    return it == m_cbs.end() ? nullptr : it->second;
  }

 private:
  bool funcIdGrowthValid(uint64_t id) {
    static const uint64_t func_id_threshold = 256;
    if (s_func_id < func_id_threshold) {
      if (id < s_func_id * 2) {
        return true;
      }
    } else {
      if (id < (s_func_id + 3 * func_id_threshold) / 4 + s_func_id) {
        return true;
      }
    }
    return false;
  }

 private:
  T m_val;
  uint64_t s_func_id = 0;
  std::unordered_map<uint64_t, Callback> m_cbs;  // 加map是因为回调函数没有等于
};

template <class T>
std::ostream& operator<<(std::ostream& os, const ConfigVar<T>& var) {
  os << var.toString();
  return os;
}

// Config
class Config {
 public:
  using ConfigVarMap = std::unordered_map<std::string, ConfigVarBase::ptr>;
  using RWMutexType = RWMutex;

  template <class T>
  using ConfigVarPtr = typename ConfigVar<T>::ptr;

  using VisitCallback = std::function<void(ConfigVarBase::ptr)>;

  static const std::string& kValidSet();

  template <class T>
  static ConfigVarPtr<T> Lookup(const std::string& name, const T& default_value,
                                const std::string& description = "") {
    RWMutexType::WriteLock lock(GetMutex());

    auto it = GetDatas().find(name);
    if (it != GetDatas().end()) {
      auto tmp = std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
      if (tmp) {
        return tmp;
      }
      throw std::logic_error("the type not equal");
    }

    if (name.find_first_not_of(kValidSet()) != std::string::npos) {
      throw std::invalid_argument(name);
    }

    ConfigVarPtr<T> elem =
        std::make_shared<ConfigVar<T>>(name, default_value, description);
    GetDatas().insert(std::make_pair(name, elem));
    return elem;
  }

  template <class T>
  static ConfigVarPtr<T> Lookup(const std::string& name) {
    RWMutexType::ReadLock lock(GetMutex());
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

  static void Visit(VisitCallback cb);

 private:
  // static ConfigVarMap s_datas;  // 程序开始就初始化
  // 在第一次调用的时候才初始化，避免慢启动问题，控制创建时机，避免不必要的开销
  // 最主要的原因是保证static初始化元素之间没有初始化顺序，通过这样能够明确调用顺序
  static RWMutexType& GetMutex() {
    static RWMutexType m_rwmutex;
    return m_rwmutex;
  }

  static ConfigVarMap& GetDatas() {
    static ConfigVarMap s_datas;
    return s_datas;
  }
};

}  // namespace ddg

#endif
