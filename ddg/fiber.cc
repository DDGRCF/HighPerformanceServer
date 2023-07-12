#include "ddg/fiber.h"

#include <atomic>
#include <system_error>
#include <unordered_set>

#include "ddg/config.h"
#include "ddg/log.h"
#include "ddg/macro.h"
#include "ddg/scheduler.h"

namespace ddg {

static Logger::ptr g_logger = DDG_LOG_ROOT();

static thread_local Fiber* t_cur_fiber = nullptr;  // 当前正在运行的协程

static thread_local Fiber::ptr t_thread_fiber = nullptr;  // 当前线程的主协程

static std::atomic<uint64_t> s_fiber_id{0};

static std::atomic<uint64_t> s_fiber_count{0};

static ConfigVar<uint32_t>::ptr g_fiber_stack_size = Config::Lookup<uint32_t>(
    "fiber.stack_size", 128 * 1024, "fiber stack size");

void* MallocStackAllocator::Alloc(size_t size) noexcept {
  return malloc(size);
}

void* MallocStackAllocator::Realloc(void* vp, size_t size) noexcept {
  return realloc(vp, size);
}

void MallocStackAllocator::Dealloc(void* vp, size_t size) noexcept {
  free(vp);
}

std::string Fiber::State::ToString(Fiber::State::Type type) {
  switch (type) {
#define XX(name)             \
  case (Fiber::State::name): \
    return #name;            \
    break
    XX(READY);
    XX(RUNNING);
    XX(HOLD);
    XX(TERM);
#undef XX
    default:
      return "UNKNOW";
  }
}

Fiber::State::Type Fiber::State::FromString(const std::string& name) {
#define XX(type)               \
  if (#type == name) {         \
    return Fiber::State::type; \
  }
  XX(READY);
  XX(RUNNING);
  XX(HOLD);
  XX(TERM);
  return State::UNKNOW;
}

Fiber::Fiber() {
  SetThis(this);

  int ret = getcontext(&m_ctx);
  if (DDG_UNLIKELY(ret)) {
    DDG_ASSERT_MSG(false, "getcontext");
  }

  s_fiber_count++;
  m_state = State::RUNNING;
}

Fiber::Fiber(Callback cb, size_t stacksize) : m_cb(cb) {
  m_stacksize = stacksize ? stacksize : g_fiber_stack_size->getValue();

  int ret = getcontext(&m_ctx);
  if (DDG_UNLIKELY(ret)) {
    DDG_ASSERT_MSG(false, "getcontext");
  }

  m_stack = StackAllocator::Alloc(m_stacksize);  // 分配内存

  // 指向当前上下文接收后，下一个激活的上下文对象指针，只在当前上下文有makecontext创建有效
  m_ctx.uc_link = nullptr;
  // 存放上下文的栈
  m_ctx.uc_stack.ss_sp = m_stack;
  // 栈的大小
  m_ctx.uc_stack.ss_size = m_stacksize;

  // 将上下文和主回调绑定
  makecontext(&m_ctx, &Fiber::MainCallback, 0);  // 使用mainfiber回调

  s_fiber_count++;
  s_fiber_id++;
  m_id = s_fiber_id;
}

Fiber::~Fiber() {
  if (m_stack) {  // 有栈，说明不是主协程
    DDG_ASSERT(m_state == State::TERM);
    StackAllocator::Dealloc(m_stack, m_stacksize);
  } else {  // 没有栈，说明是主协程
    DDG_ASSERT(m_cb.getCallback() == nullptr);
    DDG_ASSERT(m_state == State::RUNNING);

    // 这一步没有影响
    Fiber* cur = t_cur_fiber;  // 当前协程就是自己
    if (cur == this) {  // 如果当前协程就是自己就退出  TODO: 这里可以不要
      SetThis(nullptr);
    }
  }

  m_state = State::TERM;
  s_fiber_count--;
}

void Fiber::reset(Callback cb) {
  // 只有主协程没有栈，非主协程才可以回调
  DDG_ASSERT(m_stack);
  DDG_ASSERT(m_state == State::TERM);

  // 重新设置回调函数
  m_cb = cb;

  int ret = getcontext(&m_ctx);
  if (DDG_UNLIKELY(ret)) {
    DDG_ASSERT_MSG(false, "getcontext");
  }

  m_ctx.uc_link = nullptr;
  m_ctx.uc_stack.ss_sp = m_stack;
  m_ctx.uc_stack.ss_size = m_stacksize;

  makecontext(&m_ctx, &Fiber::MainCallback, 0);

  m_state = State::READY;
}

uint64_t Fiber::getId() const {
  return m_id;
}

Fiber::State::Type Fiber::getState() const {
  return m_state;
}

void Fiber::setState(Fiber::State::Type state) {
  MutexType::Lock lock(m_mutex);
  m_state = state;
}

void Fiber::SetThis(Fiber* f) {
  t_cur_fiber = f;
}

Fiber::ptr Fiber::GetThis() {
  if (t_cur_fiber) {  // 如果当前线程存在，就返回
    return t_cur_fiber->shared_from_this();
  }

  // 如果当前线程不存在则，主线程一定不存在，这时候需要创造主线程
  Fiber::ptr main_fiber(new Fiber());
  DDG_ASSERT(t_cur_fiber == main_fiber.get());

  t_thread_fiber = main_fiber;
  return t_cur_fiber->shared_from_this();
}

// 让出
void Fiber::yield(Fiber::State::Type type) {
  DDG_ASSERT(m_state == State::RUNNING ||
             m_state == State::TERM);  // 终止和运行状态可以

  SetThis(t_thread_fiber.get());

  if (m_state != State::TERM) {
    m_state = type;
  }

  if (DDG_UNLIKELY(swapcontext(&m_ctx, &t_thread_fiber->m_ctx))) {
    DDG_ASSERT_MSG(false, "swapcontext");
    m_state = State::TERM;
  }
}

void Fiber::resume() {
  DDG_ASSERT(m_state == State::READY);

  SetThis(this);

  m_state = State::RUNNING;

  if (DDG_UNLIKELY(swapcontext(&t_thread_fiber->m_ctx, &m_ctx))) {
    DDG_ASSERT_MSG(false, "swapcontext");
    m_state = State::TERM;
  }
}

void Fiber::Yield(Fiber::State::Type type) {
  DDG_ASSERT(type == Fiber::State::HOLD || type == Fiber::State::READY);
  Fiber::ptr cur = GetThis();
  cur->yield(type);
}

void Fiber::YieldToHold() {
  Yield(Fiber::State::HOLD);
}

uint64_t Fiber::GetFiberNum() {
  return s_fiber_count;
}

void Fiber::MainCallback() {
  Fiber::ptr cur = GetThis();

  DDG_ASSERT(cur);

  cur->m_cb();
  cur->m_cb = nullptr;

  auto raw_ptr = cur.get();
  cur.reset();
  raw_ptr->yield();
}

uint64_t Fiber::GetFiberId() {
  if (t_cur_fiber) {
    return t_cur_fiber->getId();
  }
  return ~0ull;
}

}  // namespace ddg
