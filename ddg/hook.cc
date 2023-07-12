#include "ddg/hook.h"

#include <arpa/inet.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "ddg/config.h"
#include "ddg/fd_manager.h"
#include "ddg/fiber.h"
#include "ddg/iomanager.h"
#include "ddg/log.h"
#include "ddg/macro.h"

#define HOOK_FUN(XX) \
  XX(sleep)          \
  XX(usleep)         \
  XX(nanosleep)      \
  XX(fcntl)          \
  XX(connect)        \
  XX(read)           \
  XX(accept)         \
  XX(readv)          \
  XX(recv)           \
  XX(recvfrom)       \
  XX(ioctl)          \
  XX(setsockopt)     \
  XX(getsockopt)     \
  XX(socket)         \
  XX(recvmsg)        \
  XX(write)          \
  XX(writev)         \
  XX(send)           \
  XX(sendto)         \
  XX(sendmsg)        \
  XX(close)

namespace ddg {

static Logger::ptr g_logger = DDG_LOG_ROOT();

static ddg::ConfigVar<time_t>::ptr g_tcp_connect_timeout =
    ddg::Config::Lookup("tcp.connect.timeout", 5000l, "tcp connext timeout");

static time_t s_connect_timeout = -1;

static thread_local bool t_hook_enable = false;

static void hook_init() {
  static bool is_inited = false;
  if (is_inited) {
    return;
  }
#define XX(name) \
  name##_f = reinterpret_cast<name##_func>(dlsym(RTLD_NEXT, #name));
  HOOK_FUN(XX);
#undef XX
  is_inited = true;  // 补充
}

bool is_hook_enable() {
  return t_hook_enable;
}

void set_hook_enable(bool flag) {
  t_hook_enable = flag;
}

// 在命名空间中防止重名
namespace {
struct _HookIniter {
  _HookIniter() {
    hook_init();
    s_connect_timeout = g_tcp_connect_timeout->getValue();

    g_tcp_connect_timeout->addListener(
        [](const int& old_value, const int& new_value) {
          DDG_LOG_INFO(g_logger) << "tcp connect timeout changed from "
                                 << old_value << " to " << new_value;
          s_connect_timeout = new_value;
        });
  }
};

static _HookIniter s_hook_initer;
}  // namespace

}  // namespace ddg

struct timer_info {
  int cancelled = 0;
};

template <typename OriginFun, typename... Args>
static ssize_t do_io(int fd, OriginFun fun, const char* hook_fun_name,
                     uint32_t event, int timeout_so, Args&&... args) {
  if (!ddg::t_hook_enable) {
    return fun(fd, std::forward<Args>(args)...);
  }

  ddg::FdContext::ptr ctx = ddg::FdMgr::GetInstance()->get(fd);
  if (!ctx) {
    return fun(fd, std::forward<Args>(args)...);
  }

  if (ctx->isClose()) {
    errno = EBADF;
    return -1;
  }

  if (!ctx->isSocket() || ctx->getUserNonblock()) {
    return fun(fd, std::forward<Args>(args)...);
  }

  time_t to = ctx->getTimeout(timeout_so);
  std::shared_ptr<timer_info> tinfo(new timer_info);

retry:
  ssize_t ret = fun(fd, std::forward<Args>(args)...);
  while (ret == -1 && errno == EINTR) {
    ret = fun(fd, std::forward<Args>(args)...);
  }

  if (ret == -1 && errno == EAGAIN) {
    ddg::IOManager* iom = ddg::IOManager::GetThis();
    ddg::Timer::ptr timer;
    std::weak_ptr<timer_info> winfo(tinfo);

    if (to != -1) {
      timer = iom->addConditionTimer(
          to,
          [winfo, fd, iom, event]() {
            auto t = winfo.lock();
            if (!t || t->cancelled) {
              return;
            }
            t->cancelled = ETIMEDOUT;
            iom->cancelEvent(fd,
                             static_cast<ddg::IOManager::Event::Type>(event));
          },
          winfo);
    }

    if (DDG_LIKELY(iom->addEvent(
            fd, static_cast<ddg::IOManager::Event::Type>(event)))) {
      ddg::Fiber::YieldToHold();
      if (timer) {
        timer->cancel();
      }
      if (tinfo->cancelled) {
        errno = tinfo->cancelled;
        return -1;
      }
      goto retry;
    } else {
      DDG_LOG_ERROR(ddg::g_logger) << hook_fun_name << " IOManager::addEvent("
                                   << fd << ", " << event << ")";
      if (timer) {
        timer->cancel();
      }
      return -1;
    }
  }
  if (ret == -1) {
    int error = 0;
    socklen_t len = sizeof(error);
    ret = getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len);
    if (ret == -1) {
      return -1;
    }

    errno = error;
    if (!error) {
      return 0;
    } else {
      return -1;
    }
  }
  return ret;
  // 获取当前socket状态
}

extern "C" {

#define XX(name) name##_func name##_f = nullptr;
HOOK_FUN(XX);
#undef XX

unsigned int sleep(unsigned int seconds) {
  if (!ddg::t_hook_enable) {
    return sleep_f(seconds);
  }

  ddg::Fiber::ptr fiber = ddg::Fiber::GetThis();
  ddg::IOManager* iom = ddg::IOManager::GetThis();

  iom->addTimer(
      seconds * 1000,
      std::bind(
          static_cast<void (ddg::Scheduler::*)(ddg::Fiber::ptr, int thread)>(
              &ddg::IOManager::schedule),
          iom, fiber, -1));
  ddg::Fiber::YieldToHold();

  return 0;
}

int usleep(useconds_t usec) {
  if (!ddg::t_hook_enable) {
    return usleep_f(usec);
  }
  ddg::Fiber::ptr fiber = ddg::Fiber::GetThis();
  ddg::IOManager* iom = ddg::IOManager::GetThis();

  iom->addTimer(
      usec / 1000,
      std::bind(
          static_cast<void (ddg::Scheduler::*)(ddg::Fiber::ptr, int thread)>(
              &ddg::IOManager::schedule),
          iom, fiber, -1));
  ddg::Fiber::YieldToHold();
  return 0;
}

int nanosleep(const struct timespec* req, struct timespec* rem) {
  if (!ddg::t_hook_enable) {
    return nanosleep(req, rem);
  }

  ddg::Fiber::ptr fiber = ddg::Fiber::GetThis();
  ddg::IOManager* iom = ddg::IOManager::GetThis();

  uint64_t time = req->tv_sec * 1000 + req->tv_nsec / 1e6;

  iom->addTimer(
      time,
      std::bind(
          static_cast<void (ddg::Scheduler::*)(ddg::Fiber::ptr, int thread)>(
              &ddg::IOManager::schedule),
          iom, fiber, -1));
  ddg::Fiber::YieldToHold();
  return 0;
}

int socket(int domain, int type, int protocol) {
  if (!ddg::t_hook_enable) {
    return socket_f(domain, type, protocol);
  }

  int fd = socket_f(domain, type, protocol);
  if (fd == -1) {
    return fd;
  }
  ddg::FdMgr::GetInstance()->get(fd, true);
  return fd;
}

ssize_t read(int fd, void* buf, size_t count) {
  return do_io(fd, read_f, "read", ddg::IOManager::Event::READ, SO_RCVTIMEO,
               buf, count);
}

ssize_t readv(int fd, const struct iovec* iov, int iovcnt) {
  return do_io(fd, readv_f, "readv", ddg::IOManager::Event::READ, SO_RCVTIMEO,
               iov, iovcnt);
}

ssize_t recv(int sockfd, void* buf, size_t len, int flags) {
  return do_io(sockfd, recv_f, "recv", ddg::IOManager::Event::READ, SO_RCVTIMEO,
               buf, len, flags);
}

ssize_t recvfrom(int sockfd, void* buf, size_t len, int flags,
                 struct sockaddr* src_addr, socklen_t* addrlen) {
  return do_io(sockfd, recvfrom_f, "recvfrom", ddg::IOManager::Event::READ,
               SO_RCVTIMEO, buf, len, flags, src_addr, addrlen);
}

ssize_t recvmsg(int sockfd, struct msghdr* msg, int flags) {
  return do_io(sockfd, recvmsg_f, "recvmsg", ddg::IOManager::Event::READ,
               SO_RCVTIMEO, msg, flags);
}

ssize_t write(int fd, const void* buf, size_t count) {
  return do_io(fd, write_f, "write", ddg::IOManager::Event::WRITE, SO_SNDTIMEO,
               buf, count);
}

ssize_t writev(int fd, const struct iovec* iov, int iovcnt) {
  return do_io(fd, writev_f, "writev", ddg::IOManager::Event::WRITE,
               SO_SNDTIMEO, iov, iovcnt);
}

ssize_t send(int s, const void* msg, size_t len, int flags) {
  return do_io(s, send_f, "send", ddg::IOManager::Event::WRITE, SO_SNDTIMEO,
               msg, len, flags);
}

ssize_t sendto(int s, const void* msg, size_t len, int flags,
               const struct sockaddr* to, socklen_t tolen) {
  return do_io(s, sendto_f, "sendto", ddg::IOManager::Event::WRITE, SO_SNDTIMEO,
               msg, len, flags, to, tolen);
}

ssize_t sendmsg(int s, const struct msghdr* msg, int flags) {
  return do_io(s, sendmsg_f, "sendmsg", ddg::IOManager::Event::WRITE,
               SO_SNDTIMEO, msg, flags);
}

int accept(int s, struct sockaddr* addr, socklen_t* addrlen) {
  int fd = do_io(s, accept_f, "accept", ddg::IOManager::Event::READ,
                 SO_RCVTIMEO, addr, addrlen);
  if (fd >= 0) {
    ddg::FdMgr::GetInstance()->get(fd, true);
  }
  return fd;
}

int connect_with_timeout(int fd, const struct sockaddr* addr, socklen_t addrlen,
                         time_t timeout_ms) {
  if (!ddg::t_hook_enable) {
    return connect_f(fd, addr, addrlen);
  }

  ddg::FdContext::ptr ctx = ddg::FdMgr::GetInstance()->get(fd);
  if (!ctx || ctx->isClose()) {
    errno = EBADF;
    return -1;
  }

  if (!ctx->isSocket()) {
    return connect_f(fd, addr, addrlen);
  }

  // 使用用户设置的阻塞
  if (ctx->getUserNonblock()) {
    return connect_f(fd, addr, addrlen);
  }

  int ret = connect_f(fd, addr, addrlen);
  if (ret == 0) {
    return 0;
  } else if (ret != -1 || errno != EINPROGRESS) {
    return ret;
  }

  ddg::IOManager* iom = ddg::IOManager::GetThis();

  ddg::Timer::ptr timer;

  std::shared_ptr<timer_info> tinfo = std::make_shared<timer_info>();

  std::weak_ptr<timer_info> winfo(tinfo);

  if (timeout_ms != -1) {
    timer = iom->addConditionTimer(
        timeout_ms,
        [winfo, fd, iom]() {
          auto t = winfo.lock();
          if (!t || t->cancelled) {
            return;
          }
          t->cancelled = ETIMEDOUT;
          iom->cancelEvent(fd, ddg::IOManager::Event::WRITE);
        },
        winfo);
  }

  if (DDG_LIKELY(iom->addEvent(fd, ddg::IOManager::Event::WRITE))) {
    ddg::Fiber::YieldToHold();
    if (timer) {
      timer->cancel();
    }
    if (tinfo->cancelled) {
      errno = tinfo->cancelled;
      return -1;
    }
  } else {
    if (timer) {
      timer->cancel();
    }
    DDG_LOG_ERROR(ddg::g_logger) << "connect IOManager::addEvent(" << fd
                                 << ", IOManager::Event::WRITE) error";
  }

  int error = 0;  // 获取当前socket状态
  socklen_t len = sizeof(error);
  ret = getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len);
  if (ret == -1) {
    return -1;
  }

  errno = error;
  if (!error) {
    return 0;
  } else {
    return -1;
  }
}

int connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen) {
  return connect_with_timeout(sockfd, addr, addrlen, ddg::s_connect_timeout);
}

int fcntl(int fd, int cmd, ...) {
  va_list ap;
  va_start(ap, cmd);

  switch (cmd) {
    case F_SETFL: {
      int arg = va_arg(ap, int);
      va_end(ap);
      ddg::FdContext::ptr ctx = ddg::FdMgr::GetInstance()->get(fd);
      if (!ctx || ctx->isClose() || !ctx->isSocket()) {
        return fcntl_f(fd, cmd, arg);
      }

      // TODO: 感觉不太合理，如果usernonblock也设置为nonblock，则需要全部都设置
      ctx->setUserNonblock(arg & O_NONBLOCK);
      ctx->setSysNonblock(arg & O_NONBLOCK);

      if (ctx->getSysNonblock()) {
        // 只有socket状态在创建的时候设置nonblock的，因此socket状态本身就是nonblock
        arg |= O_NONBLOCK;
      } else {
        arg &= ~O_NONBLOCK;
      }
      return fcntl_f(fd, cmd, arg);
    } break;
    case F_GETFL: {
      va_end(ap);
      int arg = fcntl_f(fd, cmd);
      // autocreate false 保证不会循环创建
      ddg::FdContext::ptr ctx = ddg::FdMgr::GetInstance()->get(fd);
      if (!ctx || ctx->isClose() || !ctx->isSocket()) {
        return arg;
      }
      // hook住后主要体现用户态的nonblock
      if (ctx->getUserNonblock()) {
        // 如果是用户主动设置的O_NONBLOCK，返回应该为NONBLOCK
        return arg | O_NONBLOCK;
      } else {
        // 如果不是用户主动设置的O_NONBLOCK，返回应该不是，但实际上应该是
        return arg & ~O_NONBLOCK;
      }
    } break;
    case F_DUPFD:
    case F_DUPFD_CLOEXEC:
    case F_SETFD:
    case F_SETOWN:
    case F_SETSIG:
    case F_SETLEASE:
    case F_NOTIFY:
#ifdef F_SETPIPE_SZ
    case F_SETPIPE_SZ:
#endif
    {
      int arg = va_arg(ap, int);
      va_end(ap);
      return fcntl_f(fd, cmd, arg);
    } break;
    case F_GETFD:
    case F_GETOWN:
    case F_GETSIG:
    case F_GETLEASE:
#ifdef F_GETPIPE_SZ
    case F_GETPIPE_SZ:
#endif
    {
      va_end(ap);
      return fcntl_f(fd, cmd);
    } break;
    case F_SETLK:
    case F_SETLKW:
    case F_GETLK: {
      struct flock* arg = va_arg(ap, struct flock*);
      va_end(ap);
      return fcntl_f(fd, cmd, arg);
    } break;
    case F_GETOWN_EX:
    case F_SETOWN_EX: {
      struct f_owner_exlock* arg = va_arg(ap, struct f_owner_exlock*);
      va_end(ap);
      return fcntl_f(fd, cmd, arg);
    } break;
    default:
      va_end(ap);
      return fcntl_f(fd, cmd);
  }

  va_end(ap);
}

int ioctl(int fd, unsigned long int request, ...) {
  va_list vp;
  va_start(vp, request);
  void* arg = va_arg(vp, void*);
  va_end(vp);

  if (FIONBIO == request) {
    bool user_nonblock = !!*static_cast<int*>(arg);
    ddg::FdContext::ptr ctx = ddg::FdMgr::GetInstance()->get(fd);
    if (!ctx || ctx->isClose() || !ctx->isSocket()) {
      return ioctl_f(fd, request, arg);
    }
    ctx->setUserNonblock(user_nonblock);
  }
  return ioctl_f(fd, request, arg);
}

int setsockopt(int sockfd, int level, int optname, const void* optval,
               socklen_t optlen) {
  if (!ddg::t_hook_enable) {
    return setsockopt_f(sockfd, level, optname, optval, optlen);
  }
  if (level == SOL_SOCKET) {
    if (optname == SO_RCVTIMEO || optname == SO_SNDTIMEO) {
      ddg::FdContext::ptr ctx = ddg::FdMgr::GetInstance()->get(sockfd);
      if (ctx) {
        const timeval* v = static_cast<const timeval*>(optval);
        ctx->setTimeout(optname, v->tv_sec * 1000 + v->tv_usec / 1000);
      }
    }
  }
  return setsockopt_f(sockfd, level, optname, optval, optlen);
}

int getsockopt(int sockfd, int level, int optname, void* optval,
               socklen_t* optlen) {
  return getsockopt_f(sockfd, level, optname, optval, optlen);
}

int close(int fd) {
  if (!ddg::t_hook_enable) {
    return close_f(fd);
  }
  ddg::FdContext::ptr ctx = ddg::FdMgr::GetInstance()->get(fd);
  if (ctx) {
    auto iom = ddg::IOManager::GetThis();
    if (iom) {
      iom->cancelAll(fd);
    }
    ddg::FdMgr::GetInstance()->del(fd);
  }
  return close_f(fd);
}
}
