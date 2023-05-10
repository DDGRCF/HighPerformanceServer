#include "ddg/thread.h"
#include "ddg/log.h"

namespace ddg {

static thread_local Thread* t_thread = nullptr;
static thread_local std::string t_thread_name = "UNKNOW";
static auto g_logger = DDG_LOG_ROOT();

Thread* Thread::GetThis() {
  return t_thread;
}

std::string GetName() {
  return t_thread_name;
}

void Thread::SetName(const std::string& name) {
  if (name.empty()) {
    return;
  }
  if (t_thread) {
    t_thread->m_name = name;
  }
  t_thread_name = name;
}

Thread::Thread(const std::string& name, Callback cb) : m_cb(cb), m_name(name) {
  if (name.empty()) {
    m_name = "UNKNOW";
  }

  int ret = pthread_create(&m_thread, nullptr, &Thread::run, this);
  if (ret != 0) {
    DDG_LOG_ERROR(g_logger)
        << "pthread_create thread fail, ret = " << ret << " name = " << m_name;
    throw std::logic_error("pthread_create error");
  }
  m_sem.wait();
}

Thread::~Thread() {
  if (m_thread) {
    pthread_detach(m_thread);
  }
}

void Thread::join() {
  if (m_thread) {
    int ret = pthread_join(m_thread, nullptr);
    if (ret) {
      DDG_LOG_ERROR(g_logger)
          << "pthread_join thread fail. ret = " << ret << " name = " << m_name;
      throw std::logic_error("pthread_join error");
    }
    m_thread = 0;
  }
}

void Thread::detach() {
  if (m_thread) {
    int ret = pthread_detach(m_thread);
    if (ret) {
      DDG_LOG_ERROR(g_logger)
          << "pthread_join thread fail. ret = " << ret << " name = " << m_name;
      throw std::logic_error("pthread_join error");
    }
    m_thread = 0;
  }
}

void* Thread::run(void* arg) {
  Thread* thread = static_cast<Thread*>(arg);

  t_thread = thread;
  t_thread_name = thread->m_name;
  thread->m_id = ddg::getThreadId();
  pthread_setname_np(pthread_self(), thread->m_name.substr(0, 10).c_str());

  Callback cb;
  cb.swap(thread->m_cb);

  thread->m_sem.post();
  cb();

  return static_cast<void*>(0);
}

}  // namespace ddg
