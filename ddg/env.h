#ifndef DDG_ENV_H_
#define DDG_ENV_H_

#include <string>
#include <unordered_map>
#include <vector>
#include "ddg/singleton.h"

namespace ddg {

class Env {
 public:
  bool init(int argc, char** argv);
  void add(const std::string& key, const std::string& val);
  void has(const std::string& key);
  void del(const std::string& key);
  std::string get(const std::string& key,
                  const std::string& default_value = "");
};

#endif
