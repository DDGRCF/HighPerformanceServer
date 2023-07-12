#ifndef DDG_LEXICALCAST_H_
#define DDG_LEXICALCAST_H_

#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <yaml-cpp/yaml.h>
#include <boost/lexical_cast.hpp>

namespace ddg {
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

// for stl
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

}  // namespace ddg

#endif
