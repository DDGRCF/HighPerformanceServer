#ifndef DDG_THREAD_H_
#define DDG_THREAD_H_

#include <pthread.h>
#include <functional>
#include <memory>
#include <string>

namespace ddg {
class Thread {
 public:
  typedef std::shared_ptr<Thread> ptr;
  Thread(std::function<void> cb, const std::string& name);
  ~Thread();

  void join();
  void detach();

  static Thread* GetThis();
  static const std::string& GetName();
  static void SetName(const std::string& name);

 private:
  Thread(const Thread&) = delete;
  Thread(Thread&&) = delete;
  Thread& operator=(const Thread&) = delete;

 private:
  pid_t m_id = -1;
  pthread_t m_thread = 0;
  std::function<void()> m_cb;
  std::string m_name;
};
}  // namespace ddg

#endif
