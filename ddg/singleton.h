#ifndef DDG_SINGLETON_H_
#define DDG_SINGLETON_H_

#include <memory>

namespace ddg {

template <class T, class X = void, int N = 0>
class Singleton {
 public:
  static T* GetInstance() {
    static T v;
    return &v;
  }

  Singleton() = delete;
  ~Singleton() = delete;
  Singleton(const Singleton&) = delete;
  Singleton& operator=(const Singleton&);

 private:
};

template <class T, class X = void, int N = 0>  // TODO:
class SingletonPtr {
 public:
  static std::shared_ptr<T> GetInstance() {
    static auto v = std::make_shared<T>();
    return v;
  }

  SingletonPtr() = delete;
  ~SingletonPtr() = delete;
  SingletonPtr(const SingletonPtr&) = delete;
  SingletonPtr& operator=(const SingletonPtr&);

 private:
};

}  // namespace ddg

#endif
