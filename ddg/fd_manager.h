#ifndef DDG_FD_MANAGER_H_
#define DDG_FD_MANAGER_H_

#include <vector>

#include "ddg/mutex.h"
#include "ddg/singleton.h"
#include "ddg/thread.h"

namespace ddg {

class FdContext : public std::enable_shared_from_this<FdContext> {
 public:
  using ptr = std::shared_ptr<FdContext>;

  FdContext(int fd);

  ~FdContext();

  bool init();

  bool isInit() const;

  bool isSocket() const;

  bool isClose() const;

  void setUserNonblock(bool v);

  bool getUserNonblock() const;

  void setSysNonblock(bool v);

  bool getSysNonblock() const;

  void setTimeout(int type, time_t v);

  int getTimeout(int type);

 private:
  bool m_isinit : 1;

  bool m_issocket : 1;

  bool m_sys_nonblock : 1;

  bool m_user_nonblock : 1;

  bool m_isclosed : 1;

  int m_fd;

  time_t m_recv_timeout = -1;

  time_t m_send_timeout = -1;
};

class FdManager {
 public:
  using MutexType = SpinLock;

  FdManager();

  FdContext::ptr get(int fd, bool auto_create = false);

  void del(int fd);

 private:
  MutexType m_mutex;

  std::vector<FdContext::ptr> m_datas;
};

using FdMgr = Singleton<FdManager>;

}  // namespace ddg

#endif
