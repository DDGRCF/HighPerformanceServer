#include "ddg/fiber.h"

#include <atomic>
#include <system_error>
#include <unordered_set>

#include "ddg/config.h"
#include "ddg/log.h"
#include "ddg/macro.h"
#include "ddg/scheduler.h"

namespace ddg {

static thread_local Fiber* t_fiber = nullptr;  // 当前正在运行的协程

static thread_local Fiber::ptr t_threadFiber = nullptr;  // 当前线程的主协程

static std::atomic<uint64_t> s_fiber_id{0};

static std::atomic<uint64_t> s_fiber_count{0};

static Logger::ptr g_logger = DDG_LOG_ROOT();

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

std::string Fiber::State::ToString(State::Type type) {
  switch (type) {
#define XX(name)             \
  case (Fiber::State::name): \
    return #name;            \
    break
    XX(UNINIT);
    XX(INIT);
    XX(HOLD);
    XX(EXEC);
    XX(TERM);
    XX(READY);
    XX(EXCEPT);
#undef XX
    default:
      return "Unknow";
  }
}

Fiber::State::Type Fiber::State::FromString(const std::string& name) {
#define XX(type)               \
  if (#type == name) {         \
    return Fiber::State::type; \
  }
  XX(UNINIT);
  XX(INIT);
  XX(HOLD);
  XX(EXEC);
  XX(TERM);
  XX(READY);
  XX(EXCEPT);

  return State::UNKNOW;
}

Fiber::Fiber() {
  m_state = State::EXEC;
  SetThis(this);

  if (getcontext(&m_ctx)) {
    DDG_ASSERT_MSG(false, "getcontext");
  }

  s_fiber_count++;
  // DDG_LOG_DEBUG(g_logger) << "Fiber::Fiber main";
}

Fiber::Fiber(Callback cb, size_t stacksize, bool use_caller) : m_cb(cb) {
  m_stacksize = stacksize ? stacksize : g_fiber_stack_size->getValue();

  if (getcontext(&m_ctx)) {  // 获得当前环境的上下文
    DDG_ASSERT_MSG(false, "getcontext");
  }

  m_stack = StackAllocator::Alloc(m_stacksize);  // 分配内存

  // 指向当前上下文接收后，下一个激活的上下文对象指针，只在当前上下文有makecontext创建有效
  m_ctx.uc_link = nullptr;
  m_ctx.uc_stack.ss_sp = m_stack;
  m_ctx.uc_stack.ss_size = m_stacksize;

  if (!use_caller) {
    makecontext(&m_ctx, &Fiber::MainFunc, 0);  // 使用mainfiber回调
  } else {
    makecontext(&m_ctx, &Fiber::CallerMainFunc, 0);  // 使用当前的回调
  }

  m_state = State::INIT;
  s_fiber_count++;
  m_id = ++s_fiber_id;
  // DDG_LOG_DEBUG(g_logger) << "Fiber::Fiber id = " << m_id;
}

Fiber::~Fiber() {
  s_fiber_count--;
  if (m_stack) {  // 有栈，说明不是主协程
    DDG_ASSERT(m_state == State::TERM || m_state == State::EXCEPT ||
               m_state == State::INIT);
    StackAllocator::Dealloc(m_stack, m_stacksize);
  } else {  // 没有栈，说明是主协程
    DDG_ASSERT(m_cb == nullptr);
    DDG_ASSERT(m_state == State::EXEC);

    Fiber* cur = t_fiber;  // 当前协程就是自己
    if (cur == this) {  // 如果当前协程就是自己就退出  TODO: 这里可以不要
      SetThis(nullptr);
    }
  }
  // DDG_LOG_DEBUG(g_logger) << "Fiber::~Fiber id = " << m_id
  //                        << " total = " << s_fiber_count;
}

void Fiber::reset(Callback cb) {
  DDG_ASSERT(m_stack);
  DDG_ASSERT(m_state == State::TERM || m_state == State::EXCEPT ||
             m_state == State::INIT);
  m_cb = cb;

  if (getcontext(&m_ctx)) {
    DDG_ASSERT_MSG(false, "getcontext");
  }

  m_ctx.uc_link = nullptr;
  m_ctx.uc_stack.ss_sp = m_stack;
  m_ctx.uc_stack.ss_size = m_stacksize;

  makecontext(&m_ctx, &Fiber::MainFunc, 0);
  m_state = State::INIT;
}

void Fiber::swapIn() {
  SetThis(this);
  DDG_ASSERT(m_state != State::EXEC);
  m_state = State::EXEC;

  if (swapcontext(&Scheduler::GetMainFiber()->m_ctx,
                  &m_ctx)) {  // 从主MainFiber调入m_ctx
    DDG_ASSERT_MSG(false, "swapcontext");
  }
}

void Fiber::swapOut() {
  SetThis(Scheduler::GetMainFiber());
  if (swapcontext(&m_ctx, &Scheduler::GetMainFiber()->m_ctx)) {
    DDG_ASSERT_MSG(false, "swapcontext")
  }
}

void Fiber::call() {
  SetThis(this);
  m_state = State::EXEC;
  if (swapcontext(&t_threadFiber->m_ctx, &m_ctx)) {
    DDG_ASSERT_MSG(false, "swapcontext");
  }
}

void Fiber::back() {
  SetThis(t_threadFiber.get());
  if (swapcontext(&m_ctx, &t_threadFiber->m_ctx)) {
    DDG_ASSERT_MSG(false, "swapcontext");
  }
}

uint64_t Fiber::getId() const {
  return m_id;
}

Fiber::State::Type Fiber::getState() const {
  return m_state;
}

void Fiber::setState(Fiber::State::Type state) {
  m_state = state;
}

void Fiber::SetThis(Fiber* f) {
  t_fiber = f;
}

Fiber::ptr Fiber::GetThis() {
  if (t_fiber) {  // 如果主线程存在，就返回主协程
    return t_fiber->shared_from_this();
  }

  Fiber::ptr main_fiber = std::shared_ptr<Fiber>();
  DDG_ASSERT(t_fiber == main_fiber.get());
  t_threadFiber = main_fiber;
  return t_fiber->shared_from_this();
}

void Fiber::Yield() {
  YieldToReady();
}

void Fiber::YieldToReady() {
  Fiber::ptr cur = GetThis();
  DDG_ASSERT(cur->m_state == State::EXEC);
  cur->m_state = State::READY;
  cur->swapOut();
}

void Fiber::YieldToHold() {
  Fiber::ptr cur = GetThis();
  DDG_ASSERT(cur->m_state == State::EXEC);
  cur->m_state = State::HOLD;
  cur->swapOut();
}

void Fiber::YieldToFibers() {}

uint64_t Fiber::TotalFibers() {
  return s_fiber_count;
}

void Fiber::MainFunc() {
  Fiber::ptr cur = GetThis();

  DDG_ASSERT(cur);
  try {
    cur->m_cb();
    cur->m_cb = nullptr;
    cur->m_state = State::TERM;
  } catch (std::exception& e) {
    cur->m_state = State::EXCEPT;
    DDG_LOG_ERROR(g_logger) << "Fiber::MainFunc Except: " << e.what()
                            << " fiber_id = " << cur->getId() << "\n"
                            << ddg::BacktraceToString();
  } catch (...) {
    cur->m_state = State::EXCEPT;
    DDG_LOG_ERROR(g_logger)
        << "Fiber::MainFunc fiber_id = " << cur->getId() << "\n"
        << ddg::BacktraceToString();
  }

  auto raw_ptr = cur.get();
  cur.reset();  // 释放对象所有权
  raw_ptr->swapOut();
  DDG_ASSERT_MSG(false,
                 "never reach fiber_id = " + std::to_string(raw_ptr->getId()));
}

void Fiber::CallerMainFunc() {
  Fiber::ptr cur = GetThis();
  DDG_ASSERT(cur);

  try {
    cur->m_cb();
    cur->m_cb = nullptr;
    cur->m_state = State::TERM;
  } catch (std::exception& e) {
    cur->m_state = State::EXCEPT;
    DDG_LOG_ERROR(g_logger) << "Fiber::MainFunc Except: " << e.what()
                            << " fiber_id = " << cur->getId() << "\n"
                            << ddg::BacktraceToString();
  } catch (...) {
    cur->m_state = State::EXCEPT;
    DDG_LOG_ERROR(g_logger)
        << "Fiber::MainFunc fiber_id = " << cur->getId() << "\n"
        << ddg::BacktraceToString();
  }

  auto raw_ptr = cur.get();
  cur.reset();  // 释放对象所有权
  raw_ptr->back();
  DDG_ASSERT_MSG(false,
                 "never reach fiber_id = " + std::to_string(raw_ptr->getId()));
}

uint64_t Fiber::GetFiberId() {
  if (t_fiber) {
    return t_fiber->getId();
  }

  return 0;
}

}  // namespace ddg
