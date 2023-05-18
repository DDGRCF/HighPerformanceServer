#include "ddg/fiber.h"

#include <atomic>
#include <system_error>
#include <unordered_set>

#include "ddg/config.h"
#include "ddg/log.h"
#include "ddg/macro.h"
#include "ddg/scheduler.h"

namespace ddg {

static thread_local Fiber* t_fiber = nullptr;
static thread_local Fiber::ptr t_threadFiber = nullptr;

const uint64_t kMaxFiberId = UINT64_MAX - 1;

static std::atomic<uint64_t> s_fiber_id{0};
static std::unordered_set<uint64_t> s_fiber_id_set;

static std::atomic<uint64_t> s_fiber_count{0};

static Logger::ptr g_logger = DDG_LOG_NAME("system");

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

Fiber::Fiber() {
  m_state = EXEC;
  SetThis(this);
  if (getcontext(&m_ctx)) {
    DDG_ASSERT_MSG(false, "getcontext");
  }

  safeFiberCountIncr();  // 未设置m_stack表示当前的线程的栈区

  DDG_LOG_DEBUG(g_logger) << "Fiber::Fiber main";
}

Fiber::Fiber(Callback cb, size_t stacksize, bool use_caller) : m_cb(cb) {
  safeFiberIdIncr();
  safeFiberCountIncr();

  m_stacksize = stacksize ? stacksize : g_fiber_stack_size->getValue();

  if (getcontext(&m_ctx)) {
    DDG_ASSERT_MSG(false, "getcontext");
  }

  m_stack = StackAllocator::Alloc(m_stacksize);

  m_ctx.uc_link = nullptr;
  m_ctx.uc_stack.ss_sp = m_stack;
  m_ctx.uc_stack.ss_size = m_stacksize;

  if (!use_caller) {
    makecontext(&m_ctx, &Fiber::MainFunc, 0);  // 使用mainfiber回调
  } else {
    makecontext(&m_ctx, &Fiber::CallerMainFunc, 0);  // 使用当前的回调
  }

  DDG_LOG_DEBUG(g_logger) << "Fiber::Fiber id = " << m_id;
  m_state = INIT;
}

Fiber::~Fiber() {
  safeFiberCountDesc();
  if (m_stack) {
    DDG_ASSERT(m_state == TERM || m_state == EXCEPT || m_state == INIT);
    StackAllocator::Dealloc(m_stack, m_stacksize);
  } else {
    DDG_ASSERT(m_cb == nullptr && m_state == EXEC);
    Fiber* cur = t_fiber;

    if (cur == this) {
      SetThis(nullptr);
    }
  }

  DDG_LOG_DEBUG(g_logger) << "Fiber::~Fiber id = " << m_id
                          << " total = " << s_fiber_count;
}

void Fiber::reset(Callback cb) {
  DDG_ASSERT(m_stack);
  DDG_ASSERT(m_state == TERM || m_state == EXCEPT || m_state == INIT);
  m_cb = cb;

  if (getcontext(&m_ctx)) {
    DDG_ASSERT_MSG(false, "getcontext");
  }
  m_ctx.uc_link = nullptr;
  m_ctx.uc_stack.ss_sp = m_stack;
  m_ctx.uc_stack.ss_size = m_stacksize;

  makecontext(&m_ctx, &Fiber::MainFunc, 0);
  m_state = INIT;
}

void Fiber::swapIn() {
  SetThis(this);
  DDG_ASSERT(m_state != EXEC);
  m_state = EXEC;

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
  m_state = EXEC;
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

Fiber::State Fiber::getState() const {
  return m_state;
}

void Fiber::setState(Fiber::State state) {
  m_state = state;
}

void Fiber::SetThis(Fiber* f) {
  t_fiber = f;
}

Fiber::ptr Fiber::GetThis() {
  if (t_fiber) {
    return t_fiber->shared_from_this();
  }

  Fiber::ptr main_fiber(new Fiber);
  DDG_ASSERT(t_fiber == main_fiber.get());
  t_threadFiber = main_fiber;
  return t_fiber->shared_from_this();
}

void Fiber::Yield() {
  YieldToReady();
}

void Fiber::YieldToReady() {
  Fiber::ptr cur = GetThis();
  DDG_ASSERT(cur->m_state == EXEC);
  cur->m_state = READY;
  cur->swapOut();
}

void Fiber::YieldToHold() {
  Fiber::ptr cur = GetThis();
  cur->m_state = HOLD;
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
    cur->m_state = TERM;
  } catch (std::exception& e) {
    cur->m_state = EXCEPT;
    DDG_LOG_ERROR(g_logger) << "Fiber::MainFunc Except: " << e.what()
                            << " fiber_id = " << cur->getId() << "\n"
                            << ddg::BacktraceToString();
  } catch (...) {
    cur->m_state = EXCEPT;
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
    cur->m_state = TERM;
  } catch (std::exception& e) {
    cur->m_state = EXCEPT;
    DDG_LOG_ERROR(g_logger) << "Fiber::MainFunc Except: " << e.what()
                            << " fiber_id = " << cur->getId() << "\n"
                            << ddg::BacktraceToString();
  } catch (...) {
    cur->m_state = EXCEPT;
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

void Fiber::safeFiberIdIncr() {
  if (s_fiber_id <= kMaxFiberId - 1) {
    m_id = ++s_fiber_id;
    MutexType::Lock lock(m_mutex);
    s_fiber_id_set.insert(m_id);
  } else {
    MutexType::Lock lock(m_mutex);  // TODO:
    uint64_t i;
    for (i = 0; i <= kMaxFiberId; i++) {
      if (s_fiber_id_set.find(i) == s_fiber_id_set.end()) {
        m_id = i;
        s_fiber_id_set.insert(m_id);
        break;
      }
    }
    if (i == kMaxFiberId + 1) {
      DDG_ASSERT_MSG(false, "Fiber::safeFiberIdIncr too many fiber");
    }
  }
}

void Fiber::safeFiberCountIncr() {
  s_fiber_count++;
}

void Fiber::safeFiberCountDesc() {
  s_fiber_count--;
}

}  // namespace ddg
