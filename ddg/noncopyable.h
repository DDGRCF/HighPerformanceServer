#ifndef DDG_NONCOPYABLE_H_
#define DDG_NONCOPYABLE_H_

class NonCopyable {
 public:
  NonCopyable() = default;
  NonCopyable(const NonCopyable&) =
      delete;  // 调用拷贝构造函数的时候，就自动调用子类的拷贝构造函数

  NonCopyable& operator=(const NonCopyable&) = delete;
  ~NonCopyable() = default;
};

#endif
