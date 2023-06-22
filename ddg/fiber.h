#ifndef DDG_FIBER_H_
#define DDG_FIBER_H_

#include <ucontext.h>
#include <functional>
#include <memory>

#include "ddg/mutex.h"

namespace ddg {

class Scheduler;

class MallocStackAllocator {
 public:
  static void* Alloc(size_t size) noexcept;
  static void* Realloc(void* vp, size_t size) noexcept;
  static void Dealloc(void* vp, size_t size) noexcept;
};

using StackAllocator = MallocStackAllocator;

/**
 * @brief 协程类
 */
class Fiber : public std::enable_shared_from_this<Fiber> {
 public:
  friend Scheduler;

  using ptr = std::shared_ptr<Fiber>;
  using Callback = std::function<void()>;
  using MutexType = SpinLock;

  /**
   * @ brief 协程的状态
   */
  class State {
   public:
    enum Type {
      UNKNOW = -1,
      UNINIT = 0,
      INIT = 1,
      HOLD = 2,
      EXEC = 3,
      TERM = 4,
      READY = 5,
      EXCEPT = 6
    };

    static std::string ToString(State::Type type);

    static Fiber::State::Type FromString(const std::string& name);
  };

 private:
  // 每个协程的第一个构造函数
  Fiber();

 public:
  Fiber(Callback cb, size_t stacksize = 0, bool use_caller = false);

  ~Fiber();

  void reset(Callback cb);

  // 切换到运行态
  void swapIn();

  // 切换到后台
  void swapOut();

  // 使用主线程将当前的状态调用为当前线程的主协程
  void call();

  // 当前协程将切换到后台，也就访问主线程
  void back();

  uint64_t getId() const;

  State::Type getState() const;

 private:
  void setState(Fiber::State::Type state);

 public:
  static void SetThis(Fiber* f);

  static Fiber::ptr GetThis();

  static void Yield();

  static void YieldToReady();

  static void YieldToHold();

  static void YieldToFibers();

  static uint64_t TotalFibers();

  // 主线程调度函数
  static void MainFunc();

  // 当前线程调度函数
  static void CallerMainFunc();

  static uint64_t GetFiberId();

 private:
  void safeFiberIdIncr();

  void safeFiberCountIncr();

  void safeFiberCountDesc();

 private:
  MutexType m_mutex;

  uint64_t m_id = 0;

  uint32_t m_stacksize = 0;

  State::Type m_state = State::UNINIT;

  ucontext_t m_ctx;

  void* m_stack = nullptr;

  Callback m_cb = nullptr;
};

}  // namespace ddg

#endif
