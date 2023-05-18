#ifndef DDG_IOMANAGER_H
#define DDG_IOMANAGER_H

#include <memory>

#include <sys/epoll.h>
#include <unistd.h>
#include "ddg/fiber.h"
#include "ddg/log.h"
#include "ddg/scheduler.h"

namespace ddg {

class IOManager : public Scheduler {
 public:
  using ptr = std::shared_ptr<IOManager>;
  using Callback = std::function<void()>;
  using RWMutexType = RWMutex;

  enum Event {
    NONE = 0x0,
    READ = EPOLLIN,
    WRITE = EPOLLOUT,
  };

 private:
  struct FdContext {
    using MutexType = Mutex;
    using ptr = std::shared_ptr<FdContext>;

    struct EventContext {
      using Callback = std::function<void()>;
      Scheduler* scheduler = nullptr;
      Fiber::ptr fiber;
      Callback cb;
    };

    EventContext& getContext(Event event);

    void resetContext(EventContext& ctx);  // 取消事件
    void triggerEvent(Event event);        // 触发事件

    EventContext read;
    EventContext write;

    int fd = 0;

    Event events = NONE;
    MutexType mutex;
  };

 public:
  IOManager(size_t threads = 1, bool use_caller = true,
            const std::string& name = "");
  ~IOManager();

  int addEvent(int fd, Event event, Callback cb = nullptr);

  bool delEvent(int fd, Event event);

  bool cancelEvent(int fd, Event event);

  bool cancelAll(int fd);

  static IOManager* GetThis(int fd);

 protected:
  void tickle() override;

  bool isStoped() override;

  bool isStoped(uint64_t timeout);

  void idle() override;

  void contextResize(size_t size);

 private:
  int m_epfd = 0;

  int m_tickle_fds[2];

  RWMutexType m_mutex;

  std::vector<FdContext*> m_fdContext;

  std::atomic<size_t> m_pendingEventCount{0};
};

}  // namespace ddg

#endif
