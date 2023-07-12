#ifndef DDG_IOMANAGER_H
#define DDG_IOMANAGER_H

#include <memory>

#include <sys/epoll.h>
#include <unistd.h>
#include "ddg/fiber.h"
#include "ddg/log.h"
#include "ddg/scheduler.h"
#include "ddg/timer.h"

namespace ddg {

enum EpollCtlOp {};

std::ostream& operator<<(std::ostream& os, const EpollCtlOp& op);
std::ostream& operator<<(std::ostream& os, EPOLL_EVENTS events);

class IOManager : public Scheduler, public TimerManager {
 public:
  using ptr = std::shared_ptr<IOManager>;
  using Callback = std::function<void()>;
  using RWMutexType = RWMutex;

  struct Event {
    enum Type {
      UNKNOW = -1,
      NONE = 0x0,
      READ = EPOLLIN,    // 0x01
      WRITE = EPOLLOUT,  // 0x04
    };

    static std::string ToString(IOManager::Event::Type type);

    static IOManager::Event::Type FromString(const std::string& name);
  };

 private:
  struct FdEvent {
    using MutexType = Mutex;
    using Callback = std::function<void()>;

    using ptr = std::shared_ptr<FdEvent>;

    struct EventContext {
      Scheduler* scheduler = nullptr;
      Fiber::ptr fiber;
      Callback callback;
    };

    EventContext& getContext(Event::Type event);

    void resetContext(EventContext& ctx);  // 取消事件
    void triggerEvent(Event::Type event);  // 触发事件

    EventContext read;
    EventContext write;

    Event::Type events = Event::NONE;
    MutexType mutex;
    int fd = -1;
  };

 public:
  IOManager(size_t threads = 1, const std::string& name = "",
            bool use_caller = true, size_t epoll_size = 0);
  ~IOManager();

  bool hasPendingEvents() const;

  bool addEvent(int fd, Event::Type event, Callback cb = nullptr);

  bool delEvent(int fd, Event::Type event);

  bool cancelEvent(int fd, Event::Type event);

  bool cancelAll(int fd);

  static IOManager* GetThis();

 protected:
  bool stopping() override;

  bool stopping(time_t& timeout);

  void tickle() override;

  void idle() override;

  void eventResize(size_t size);

  void onTimerInsertedAtFront() override;

 private:
  int m_epfd = -1;

  int m_tickle_fds[2];

  RWMutexType m_mutex;

  std::vector<FdEvent*> m_fd_events;

  std::atomic<size_t> m_pending_event_count{0};
};

}  // namespace ddg

#endif
