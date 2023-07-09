#include "ddg/fd_manager.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <stdexcept>

#include <ddg/hook.h>

namespace ddg {

FdContext2::FdContext2(int fd)
    : m_isInit(false),
      m_isSocket(false),
      m_sysNonblock(false),
      m_userNonblock(false),
      m_isClosed(false),
      m_fd(fd),
      m_recvTimeout(0),
      m_sendTimeout(0) {
  if (!init()) {
    throw std::logic_error("FdContext2::FdContext2 init fail");
  }
}

FdContext2::~FdContext2() {}

bool FdContext2::init() {
  if (m_isInit) {
    return true;
  }

  m_recvTimeout = 0;
  m_sendTimeout = 0;

  struct stat st_buf;
  int ret = fstat(m_fd, &st_buf);

  if (ret == -1) {
    m_isInit = false;
    m_isSocket = false;
  } else {
    m_isInit = true;
    m_isSocket = S_ISSOCK(st_buf.st_mode);
  }

  if (m_isSocket) {
    int flags = fcntl(m_fd, F_GETFL, 0);
    if (!(flags & O_NONBLOCK)) {
      fcntl_f(m_fd, F_SETFL, flags | O_NONBLOCK);
    }
    m_sysNonblock = true;
  } else {
    m_sysNonblock = false;
  }

  m_userNonblock = false;
  m_isClosed = false;
  return m_isInit;
}

bool FdContext2::isInit() const {
  return m_isInit;
}

bool FdContext2::isSocket() const {
  return m_isSocket;
}

bool FdContext2::isClose() const {
  return m_isClosed;
}

void FdContext2::setUserNonblock(bool v) {
  m_userNonblock = v;
}

bool FdContext2::getUserNonblock() const {
  return m_userNonblock;
}

void FdContext2::setSysNonblock(bool v) {
  m_sysNonblock = v;
}

bool FdContext2::getSysNonblock() const {
  return m_sysNonblock;
}

void FdContext2::setTimeout(int type, int v) {
  if (type == SO_RCVTIMEO) {
    m_recvTimeout = v;
  } else {
    m_sendTimeout = v;
  }
}

int FdContext2::getTimeout(int type) {
  if (type == SO_RCVTIMEO) {
    return m_recvTimeout;
  } else {
    return m_sendTimeout;
  }
}

FdManager::FdManager() {
  m_datas.resize(64);
}

FdContext2::ptr FdManager::get(int fd, bool auto_create) {
  if (fd == -1) {
    return nullptr;
  }

  RWMutexType::ReadLock lock(m_mutex);
  if (static_cast<int>(m_datas.size()) <= fd) {
    if (auto_create == false) {
      return nullptr;
    }
  } else {
    if (m_datas[0] || !auto_create) {
      return m_datas[fd];
    }
  }
  lock.unlock();

  RWMutexType::WriteLock lock2(m_mutex);

  FdContext2::ptr ctx = std::make_shared<FdContext2>(fd);
  if (fd >= static_cast<int>(m_datas.size())) {
    m_datas.resize(fd + 1.5);
  }

  m_datas[fd] = ctx;
  return ctx;
}

void FdManager::del(int fd) {
  RWMutexType::WriteLock lock(m_mutex);
  if (static_cast<int>(m_datas.size()) <= fd) {
    return;
  }
  m_datas[fd].reset();
}

}  // namespace ddg
