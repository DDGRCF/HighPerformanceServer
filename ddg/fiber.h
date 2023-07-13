#ifndef DDG_FIBER_H_
#define DDG_FIBER_H_

#include <ucontext.h>
#include <functional>
#include <memory>

#include "ddg/macro.h"
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

class Fiber : public std::enable_shared_from_this<Fiber> {
 public:
  friend Scheduler;

  using ptr = std::shared_ptr<Fiber>;
  using Callback = std::function<void()>;
  using MutexType = SpinLock;

  class State {
   public:
    enum Type {
      UNKNOW = -1,
      READY = 1,
      RUNNING = 2,
      HOLD = 3,
      TERM = 4,
    };

    static std::string ToString(Fiber::State::Type type);

    static Fiber::State::Type FromString(const std::string& name);
  };

 private:
  class CallbackWrap {
   public:
    CallbackWrap(Callback callback) : m_callback(callback) {}

    CallbackWrap(void (*callback_ptr)()) { m_callback = callback_ptr; }

    Callback getCallback() const { return m_callback; }

    void setCallback(Callback callback) { m_callback = callback; }

    void operator()() {
      Fiber::ptr fiber = GetThis();
      DDG_ASSERT(fiber->getState() == Fiber::State::RUNNING);
      m_callback();
      fiber->setState(Fiber::State::TERM);
    }

   private:
    Callback m_callback = nullptr;
  };

 public:
  Fiber(Callback cb, size_t stacksize = 0);

  ~Fiber();

 private:
  Fiber();  // 主协程构造器

  void setState(Fiber::State::Type state);

 public:
  void reset(Callback cb);

  void yield(Fiber::State::Type type = Fiber::State::READY);

  void resume();

  pid_t getId() const;

  State::Type getState() const;

 public:
  static void SetThis(Fiber* f);

  static Fiber::ptr GetThis();

  static void Yield(Fiber::State::Type type = Fiber::State::READY);

  static void YieldToHold();

  static void MainCallback();

  static pid_t GetFiberId();

  static size_t GetFiberNum();

 private:
  MutexType m_mutex;

  pid_t m_id = -1;

  size_t m_stacksize = 0;

  State::Type m_state = State::READY;  // 创建实例就init状态

  ucontext_t m_ctx;

  void* m_stack = nullptr;

  CallbackWrap m_cb = nullptr;
};

}  // namespace ddg

#endif
