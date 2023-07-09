#ifndef DDG_FD_MANAGER_H_
#define DDG_FD_MANAGER_H_

#include <vector>

#include "ddg/mutex.h"
#include "ddg/singleton.h"
#include "ddg/thread.h"

namespace ddg {

class FdContext2 : public std::enable_shared_from_this<FdContext2> {
 public:
  using ptr = std::shared_ptr<FdContext2>;

  FdContext2(int fd);

  ~FdContext2();

  bool init();

  bool isInit() const;

  bool isSocket() const;

  bool isClose() const;

  void setUserNonblock(bool v);

  bool getUserNonblock() const;

  void setSysNonblock(bool v);

  bool getSysNonblock() const;

  void setTimeout(int type, int v);

  int getTimeout(int type);

 private:
  bool m_isInit : 1;

  bool m_isSocket : 1;

  bool m_sysNonblock : 1;

  bool m_userNonblock : 1;

  bool m_isClosed : 1;

  int m_fd;

  int m_recvTimeout;

  int m_sendTimeout;
};

class FdManager {
 public:
  using RWMutexType = RWMutex;

  FdManager();

  FdContext2::ptr get(int fd, bool auto_create = false);

  void del(int fd);

 private:
  RWMutexType m_mutex;

  std::vector<FdContext2::ptr> m_datas;
};

using FdMgr = Singleton<FdManager>;

}  // namespace ddg

#endif
