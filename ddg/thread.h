#ifndef DDG_THREAD_H_
#define DDG_THREAD_H_

#include <pthread.h>
#include <functional>
#include <memory>
#include <string>

#include "ddg/mutex.h"
#include "ddg/noncopyable.h"

namespace ddg {
class Thread : public NonCopyable {
 public:
  using ptr = std::shared_ptr<Thread>;

  using Callback = std::function<void()>;

  Thread(const std::string& name, Callback cb);

  ~Thread();

  pid_t getId() const { return m_id; }

  std::string getName() const { return m_name; }

  void join();
  void detach();

  static Thread* GetThis();
  static std::string GetName();

  static void SetName(const std::string& name);

  Thread(const Thread&) = delete;
  Thread(Thread&&) = delete;
  Thread& operator=(const Thread&) = delete;

  static void* run(void*);

 private:
  pid_t m_id = -1;
  pthread_t m_thread = 0;
  Callback m_cb;
  std::string m_name;
  Semphore m_sem;
};

}  // namespace ddg

#endif
