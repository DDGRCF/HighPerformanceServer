#include "ddg/fd_manager.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <stdexcept>

#include <ddg/hook.h>

namespace ddg {

FdContext::FdContext(int fd)
    : m_isinit(false),
      m_issocket(false),
      m_sys_nonblock(false),
      m_user_nonblock(false),
      m_isclosed(false),
      m_fd(fd),
      m_recv_timeout(-1),
      m_send_timeout(-1) {
  if (!init()) {
    throw std::logic_error("FdContext::FdContext init fail");
  }
}

FdContext::~FdContext() {}

bool FdContext::init() {
  if (m_isinit) {
    return true;
  }

  m_recv_timeout = -1;
  m_send_timeout = -1;

  struct stat st_buf;
  int ret = fstat(m_fd, &st_buf);

  if (ret == -1) {
    m_isinit = false;
    m_issocket = false;
  } else {
    m_isinit = true;
    m_issocket = S_ISSOCK(st_buf.st_mode);
  }

  if (m_issocket) {
    int flags = fcntl(m_fd, F_GETFL, 0);
    // 对于socket，如果不是O_NONBLOCK，就需要设置O_NONBLOCK
    if (!(flags & O_NONBLOCK)) {
      fcntl_f(m_fd, F_SETFL, flags | O_NONBLOCK);
    }
    // 就是fdcontext设置的sys_nonblock
    m_sys_nonblock = true;
  } else {
    m_sys_nonblock = false;
  }

  m_user_nonblock = false;
  m_isclosed = false;
  return m_isinit;
}

bool FdContext::isInit() const {
  return m_isinit;
}

bool FdContext::isSocket() const {
  return m_issocket;
}

bool FdContext::isClose() const {
  return m_isclosed;
}

void FdContext::setUserNonblock(bool v) {
  m_user_nonblock = v;
}

bool FdContext::getUserNonblock() const {
  return m_user_nonblock;
}

void FdContext::setSysNonblock(bool v) {
  m_sys_nonblock = v;
}

bool FdContext::getSysNonblock() const {
  return m_sys_nonblock;
}

void FdContext::setTimeout(int type, int v) {
  if (type == SO_RCVTIMEO) {
    m_recv_timeout = v;
  } else {
    m_send_timeout = v;
  }
}

int FdContext::getTimeout(int type) {
  if (type == SO_RCVTIMEO) {
    return m_recv_timeout;
  } else {
    return m_send_timeout;
  }
}

FdManager::FdManager() {
  m_datas.resize(64);
}

FdContext::ptr FdManager::get(int fd, bool auto_create) {
  if (fd == -1) {
    return nullptr;
  }

  RWMutexType::ReadLock lock(m_mutex);
  if (static_cast<int>(m_datas.size()) <= fd && !auto_create) {
    return nullptr;
  } else {
    if (m_datas[fd] || !auto_create) {
      return m_datas[fd];
    }
  }
  lock.unlock();

  RWMutexType::WriteLock lock2(m_mutex);

  FdContext::ptr ctx = std::make_shared<FdContext>(fd);
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
