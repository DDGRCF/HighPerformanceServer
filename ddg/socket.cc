#include "ddg/socket.h"

#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "ddg/fd_manager.h"
#include "ddg/hook.h"
#include "ddg/iomanager.h"
#include "ddg/log.h"
#include "ddg/macro.h"

namespace ddg {

static Logger::ptr g_logger = DDG_LOG_ROOT();

Socket::Socket(int family, int type, int protocol)
    : m_sock(-1),
      m_family(family),
      m_type(type),
      m_protocol(protocol),
      m_isconnected(false) {}

Socket::~Socket() {
  close();
}

time_t Socket::getSendTimeout() {
  FdContext::ptr ctx = FdMgr::GetInstance()->get(m_sock);
  if (ctx) {
    return ctx->getTimeout(SO_SNDTIMEO);
  }
  return -1;
}

void Socket::setSendTimeout(time_t v) {
  struct timeval tv {
    v / 1000, v % 1000 * 1000
  };

  setOption(SOL_SOCKET, SO_SNDTIMEO, tv);
}

time_t Socket::getRecvTimeout() {
  FdContext::ptr ctx = FdMgr::GetInstance()->get(m_sock);
  if (ctx) {
    return ctx->getTimeout(SO_RCVTIMEO);
  }
  return -1;
}

void Socket::setRecvTimeout(time_t v) {
  struct timeval tv {
    v / 1000, v % 1000 * 1000
  };

  setOption(SOL_SOCKET, SO_RCVTIMEO, tv);
}

bool Socket::getOption(int level, int option, void* result,
                       socklen_t* len) const {
  int ret = getsockopt(m_sock, level, option, result, len);
  if (ret) {
    DDG_LOG_DEBUG(g_logger)
        << "getOption sock = " << m_sock << " level = " << level << " option"
        << option << " errno = " << errno << " errstr = " << strerror(errno);
    return false;
  }
  return true;
}

bool Socket::setOption(int level, int option, const void* result,
                       socklen_t len) {
  int ret = setsockopt(m_sock, level, option, result, len);
  if (ret) {
    DDG_LOG_DEBUG(g_logger)
        << "setOption sock = " << m_sock << " level = " << level << " option"
        << option << " errno = " << errno << " errstr = " << strerror(errno);
    return false;
  }
  return true;
}

Socket::ptr Socket::accept() {
  // 服务端而言，创建一个新的socket
  Socket::ptr sock(new Socket(m_family, m_type, m_protocol));
  int newsock = ::accept(m_sock, nullptr, nullptr);
  if (newsock == -1) {
    DDG_LOG_ERROR(g_logger) << "accept(" << m_sock << ") errno = " << errno
                            << " errstr = " << strerror(errno);
    return nullptr;
  }
  // 初始化连接到的socket
  if (sock->init(newsock)) {
    return sock;
  }
  return nullptr;
}

bool Socket::bind(const Address::ptr addr) {
  // 如果无效，就创建新的socket，并初始化
  if (!isValid()) {
    newSock();
    if (DDG_UNLIKELY(!isValid())) {
      return false;
    }
  }

  if (DDG_UNLIKELY(addr->getFamily() != m_family)) {
    DDG_LOG_DEBUG(g_logger)
        << "bind sock.family(" << m_family << ") addr.family("
        << addr->getFamily() << ") not equal, addr = " << addr->toString();
    return false;
  }

  // unix address 需要额外创建
  UnixAddress::ptr uaddr = std::dynamic_pointer_cast<UnixAddress>(addr);
  if (uaddr) {
    Socket::ptr sock = Socket::CreateUnixTcpSocket();
    if (sock->connect(uaddr)) {
      return false;
    } else {
      // TODO: 在utils重新定义一个
      ::unlink(uaddr->getPath().c_str());
    }
  }

  if (::bind(m_sock, addr->getAddr(), addr->getAddrLen())) {
    DDG_LOG_DEBUG(g_logger)
        << "bind error errno = " << errno << " errstr = " << strerror(errno);
    return false;
  }
  getLocalAddress();
  return true;
}

bool Socket::reconnect(time_t timeout_ms) {
  if (!m_remote_address) {
    DDG_LOG_DEBUG(g_logger) << "reconnect m_remote_address is null";
    return false;
  }
  m_local_address.reset();
  return connect(m_remote_address, timeout_ms);
}

// 每次开始之前先检查一下是否初始化
// 每次查看创建的socket的family和addr是否相同
// 对于有定时需求的就使用重写的connect_with_timeout
bool Socket::connect(const Address::ptr addr, time_t timeout_ms) {
  if (!isValid()) {
    newSock();
    if (DDG_UNLIKELY(!isValid())) {
      return false;
    }
  }

  if (DDG_UNLIKELY(addr->getFamily() != m_family)) {
    DDG_LOG_DEBUG(g_logger)
        << "connect sock.family(" << m_family << ") addr.family("
        << addr->getFamily() << ") not equal, addr = " << addr->toString();
    return false;
  }

  if (timeout_ms == -1) {
    if (::connect(m_sock, addr->getAddr(), addr->getAddrLen())) {
      DDG_LOG_DEBUG(g_logger)
          << "sock = " << m_sock << " connect(" << addr->toString()
          << ") error errno = " << errno << " errstr = " << strerror(errno);
      close();
      return false;
    }
  } else {
    if (::connect_with_timeout(m_sock, addr->getAddr(), addr->getAddrLen(),
                               timeout_ms)) {
      DDG_LOG_DEBUG(g_logger)
          << "sock = " << m_sock << " connect(" << addr->toString()
          << ") timeout = " << timeout_ms << " error errno = " << errno
          << " errstr = " << strerror(errno);
      close();
      return false;
    }
  }

  m_isconnected = true;
  getRemoteAddress();
  getLocalAddress();
  return true;
}

bool Socket::listen(int backlog) {
  if (!isValid()) {
    return false;
  }

  if (::listen(m_sock, backlog)) {
    return false;
  }
  return true;
}

bool Socket::close() {
  if (!m_isconnected && m_sock == -1) {
    return true;
  }

  m_isconnected = false;
  if (m_sock != -1) {
    ::close(m_sock);
    m_sock = -1;
  }
  return false;
}

int Socket::send(const void* buffer, size_t length, int flags) {
  if (isConnected()) {
    return ::send(m_sock, buffer, length, flags);
  }
  return -1;
}

int Socket::send(const iovec* buffers, size_t length, int flags) {
  if (isConnected()) {
    msghdr msg;
    bzero(&msg, sizeof(msg));
    msg.msg_iov = const_cast<iovec*>(buffers);
    msg.msg_iovlen = length;
    return ::sendmsg(m_sock, &msg, flags);
  }
  return -1;
}

int Socket::sendTo(const void* buffer, size_t length, const Address::ptr to,
                   int flags) {
  if (isConnected()) {
    return ::sendto(m_sock, buffer, length, flags, to->getAddr(),
                    to->getAddrLen());
  }
  return -1;
}

int Socket::sendTo(const iovec* buffers, size_t length, const Address::ptr to,
                   int flags) {
  if (isConnected()) {
    msghdr msg;
    bzero(&msg, sizeof(msg));
    msg.msg_iov = const_cast<iovec*>(buffers);
    msg.msg_iovlen = length;
    msg.msg_name = const_cast<sockaddr*>(to->getAddr());
    msg.msg_namelen = to->getAddrLen();
    return ::sendmsg(m_sock, &msg, flags);
  }
  return -1;
}

int Socket::recv(void* buffer, size_t length, int flags) {
  if (isConnected()) {
    return ::recv(m_sock, buffer, length, flags);
  }
  return -1;
}

int Socket::recv(iovec* buffers, size_t length, int flags) {
  if (isConnected()) {
    msghdr msg;
    bzero(&msg, sizeof(msg));
    msg.msg_iov = const_cast<iovec*>(buffers);
    msg.msg_iovlen = length;
    return ::recvmsg(m_sock, &msg, flags);
  }
  return -1;
}

int Socket::recvFrom(void* buffer, size_t length, Address::ptr from,
                     int flags) {
  if (isConnected()) {
    socklen_t len = from->getAddrLen();
    return ::recvfrom(m_sock, buffer, length, flags,
                      const_cast<sockaddr*>(from->getAddr()), &len);
  }
  return -1;
}

int Socket::recvFrom(iovec* buffers, size_t length, Address::ptr from,
                     int flags) {
  if (isConnected()) {
    msghdr msg;
    bzero(&msg, sizeof(msg));
    msg.msg_iov = const_cast<iovec*>(buffers);
    msg.msg_iovlen = length;
    msg.msg_name = const_cast<sockaddr*>(from->getAddr());
    msg.msg_namelen = from->getAddrLen();
    return ::recvmsg(m_sock, &msg, flags);
  }
  return -1;
}

Address::ptr Socket::getRemoteAddress() {
  if (m_remote_address) {
    return m_remote_address;
  }

  Address::ptr result;
  switch (m_family) {
    case AF_INET:
      result.reset(new IPv4Address());
      break;
    case AF_INET6:
      result.reset(new IPv6Address());
      break;
    case AF_UNIX:
      result.reset(new UnixAddress());
      break;
    default:
      result.reset(new UnknownAddress(m_family));
      break;
  }
  socklen_t addrlen = result->getAddrLen();
  if (getpeername(m_sock, const_cast<sockaddr*>(result->getAddr()), &addrlen)) {
    return Address::ptr(new UnknownAddress(m_family));
  }

  if (m_family == AF_UNIX) {
    UnixAddress::ptr addr = std::dynamic_pointer_cast<UnixAddress>(result);
    addr->setAddrLen(addrlen);
  }
  m_remote_address = result;
  return m_remote_address;
}

Address::ptr Socket::getLocalAddress() {
  if (m_local_address) {
    return m_local_address;
  }

  Address::ptr result;
  switch (m_family) {
    case AF_INET:
      result.reset(new IPv4Address());
      break;
    case AF_INET6:
      result.reset(new IPv6Address());
      break;
    case AF_UNIX:
      result.reset(new UnixAddress);
      break;
    default:
      result.reset(new UnknownAddress(m_family));
      break;
  }
  socklen_t addrlen = result->getAddrLen();
  if (getsockname(m_sock, const_cast<sockaddr*>(result->getAddr()), &addrlen)) {
    DDG_LOG_DEBUG(g_logger)
        << "getsockname error sock = " << m_sock << " errno = " << errno
        << " errstr = " << strerror(errno);
    return std::make_shared<UnknownAddress>(m_family);
  }

  if (m_family == AF_UNIX) {
    UnixAddress::ptr addr = std::dynamic_pointer_cast<UnixAddress>(result);
    addr->setAddrLen(addrlen);
  }
  m_local_address = result;
  return m_local_address;
}

bool Socket::init(int sock) {
  FdContext::ptr ctx = FdMgr::GetInstance()->get(sock);
  if (!ctx || !ctx->isSocket() || ctx->isClose()) {
    DDG_LOG_DEBUG(g_logger)
        << "init(" << sock << ")"
        << " isSocket = " << std::boolalpha << ctx->isSocket()
        << " isClose = " << std::boolalpha << ctx->isClose();
    return false;
  }
  m_sock = sock;
  m_isconnected = true;
  initSock();
  getLocalAddress();
  getRemoteAddress();
  return true;
}

// 首先设置地址复用，防止2MSL等待时间过长
// 如果是TCP使用TCP_NODELAY，就是取消Nagle算法
void Socket::initSock() {
  setOption(SOL_SOCKET, SO_REUSEADDR, 1);
  if (m_type == SOCK_STREAM) {
    setOption(IPPROTO_TCP, TCP_NODELAY, 1);
  }
}

void Socket::newSock() {
  m_sock = socket(m_family, m_type, m_protocol);
  if (DDG_LIKELY(m_sock != -1)) {
    initSock();
  } else {
    DDG_LOG_DEBUG(g_logger)
        << "socket(" << m_family << ", " << m_type << ", " << m_protocol
        << ") errno = " << errno << " errstr = " << strerror(errno);
  }
}

int Socket::getFamily() const {
  return m_family;
}

int Socket::getType() const {
  return m_type;
}

int Socket::getProtocal() const {
  return m_protocol;
}

bool Socket::isConnected() const {
  return m_isconnected;
}

bool Socket::isValid() const {
  return m_sock != -1;
}

int Socket::getError() const {
  int error = 0;
  socklen_t len = sizeof(error);
  if (!getOption(SOL_SOCKET, SO_ERROR, &error, &len)) {
    error = errno;
  }
  return error;
}

std::ostream& Socket::dump(std::ostream& os) const {
  os << "[Socket] sock = " << m_sock << " is_connected = " << m_isconnected
     << " family = " << m_family << " type = " << m_type
     << " protocol = " << m_protocol;
  if (m_local_address) {
    os << " local address = " << m_local_address->toString();
  }

  if (m_remote_address) {
    os << " remote address = " << m_remote_address->toString();
  }
  return os;
}

std::string Socket::toString() const {
  std::stringstream ss;
  dump(ss);
  return ss.str();
}

std::ostream& operator<<(std::ostream& os, const Socket& sock) {
  return sock.dump(os);
}

int Socket::getSocket() const {
  return m_sock;
}

bool Socket::cancelRead() {
  return IOManager::GetThis()->cancelEvent(m_sock, ddg::IOManager::Event::READ);
}

bool Socket::cancelWrite() {
  return IOManager::GetThis()->cancelEvent(m_sock,
                                           ddg::IOManager::Event::WRITE);
}

bool Socket::cancelAccept() {
  return IOManager::GetThis()->cancelEvent(m_sock, ddg::IOManager::Event::READ);
}

bool Socket::cancelAll() {
  return IOManager::GetThis()->cancelAll(m_sock);
}

Socket::ptr Socket::CreateTcp(ddg::Address::ptr address) {
  Socket::ptr sock(new Socket(address->getFamily(), TCP, 0));
  return sock;
}

Socket::ptr Socket::CreateUdp(ddg::Address::ptr address) {
  Socket::ptr sock(new Socket(address->getFamily(), UDP, 0));
  return sock;
}

Socket::ptr Socket::CreateTcpSocket() {
  Socket::ptr sock(new Socket(IPv4, TCP, 0));
  return sock;
}

Socket::ptr Socket::CreateUdpSocket() {
  Socket::ptr sock(new Socket(IPv4, UDP, 0));
  sock->newSock();
  sock->m_isconnected = true;
  return sock;
}

Socket::ptr Socket::CreateTcpSocket6() {
  Socket::ptr sock(new Socket(IPv6, TCP, 0));
  sock->newSock();
  sock->m_isconnected = true;
  return sock;
}

Socket::ptr Socket::CreateUdpSocket6() {
  Socket::ptr sock(new Socket(IPv6, UDP, 0));
  sock->newSock();
  sock->m_isconnected = true;
  return sock;
}

Socket::ptr Socket::CreateUnixTcpSocket() {
  Socket::ptr sock(new Socket(UNIX, TCP, 0));
  return sock;
}

Socket::ptr Socket::CreateUnixUdpSocket() {
  Socket::ptr sock(new Socket(UNIX, UDP, 0));
  return sock;
}

SSLSocket::SSLSocket(int family, int type, int protocol)
    : Socket(family, type, protocol) {}

Socket::ptr SSLSocket::accept() {
  SSLSocket::ptr sock(new SSLSocket(m_family, m_type, m_protocol));
  int newsock = ::accept(m_sock, nullptr, nullptr);
  if (newsock == -1) {
    DDG_LOG_ERROR(g_logger) << "accept(" << m_sock << ") errno = " << errno
                            << " errstr = " << strerror(errno);
    return nullptr;
  }
  sock->m_ctx = m_ctx;
  if (sock->init(newsock)) {
    return sock;
  }
  return nullptr;
}

bool SSLSocket::listen(int backlog) {
  return Socket::listen(backlog);
}

bool SSLSocket::close() {
  return Socket::close();
}

bool SSLSocket::bind(const Address::ptr addr) {
  return Socket::bind(addr);
}

bool SSLSocket::connect(const Address::ptr addr, time_t timeout_ms) {
  bool v = Socket::connect(addr, timeout_ms);
  if (v) {
    m_ctx.reset(SSL_CTX_new(SSLv23_client_method()), SSL_CTX_free);
    m_ssl.reset(SSL_new(m_ctx.get()), SSL_free);
    SSL_set_fd(m_ssl.get(), m_sock);
    v = SSL_connect(m_ssl.get()) == 1;
  }
  return v;
}

int SSLSocket::send(const void* buffer, size_t length, int flags) {
  if (m_ssl) {
    return SSL_write(m_ssl.get(), buffer, length);
  }
  return -1;
}

int SSLSocket::send(const iovec* buffers, size_t length, int flags) {
  if (!m_ssl) {
    return -1;
  }
  int total = 0;
  for (size_t i = 0; i < length; i++) {
    int tmp = SSL_write(m_ssl.get(), buffers[i].iov_base, buffers[i].iov_len);
    if (tmp <= 0) {
      return tmp;
    }
    total += tmp;
    if (tmp != static_cast<int>(buffers[i].iov_len)) {
      break;
    }
  }
  return total;
}

int SSLSocket::sendTo(const void* buffer, size_t length, const Address::ptr to,
                      int flags) {
  throw std::logic_error("not support sendTo in openssl");
  return -1;
}

int SSLSocket::sendTo(const iovec* buffers, size_t length,
                      const Address::ptr to, int flags) {
  throw std::logic_error("not support sendTo in openssl");
  return -1;
}

int SSLSocket::recv(void* buffer, size_t length, int flags) {
  if (m_ssl) {
    return SSL_read(m_ssl.get(), buffer, length);
  }
  return -1;
}

int SSLSocket::recv(iovec* buffers, size_t length, int flags) {
  if (!m_ssl) {
    return -1;
  }

  int total = 0;
  for (size_t i = 0; i < length; i++) {
    int tmp = SSL_read(m_ssl.get(), buffers[i].iov_base, buffers[i].iov_len);
    if (tmp <= 0) {
      return tmp;
    }
    total += tmp;
    if (tmp != static_cast<int>(buffers[i].iov_len)) {
      break;
    }
  }
  return total;
}

int SSLSocket::recvFrom(void* buffer, size_t length, Address::ptr from,
                        int flags) {
  throw std::logic_error("not support recvfrom in openssl");
  return -1;
}

int SSLSocket::recvFrom(iovec* buffers, size_t lengt, Address::ptr from,
                        int flags) {
  throw std::logic_error("not support recvfrom in openssl");
  return -1;
}

bool SSLSocket::init(int sock) {
  bool v = Socket::init(sock);
  if (v) {
    m_ssl.reset(SSL_new(m_ctx.get()), SSL_free);
    SSL_set_fd(m_ssl.get(), m_sock);
    v = SSL_accept(m_ssl.get()) == 1;
  }
  return v;
}

bool SSLSocket::loadCertificates(const std::string& cert_file,
                                 const std::string& key_file) {
  m_ctx.reset(SSL_CTX_new(SSLv23_client_method()), SSL_CTX_free);
  if (SSL_CTX_use_certificate_chain_file(m_ctx.get(), cert_file.c_str()) != 1) {
    DDG_LOG_DEBUG(g_logger)
        << "SSL_CTX_use_certificate_chain_file(" << cert_file << ") error";
  }
  return false;
  if (SSL_CTX_use_PrivateKey_file(m_ctx.get(), key_file.c_str(),
                                  SSL_FILETYPE_PEM) != 1) {
    DDG_LOG_DEBUG(g_logger)
        << "SSL_CTX_use_PrivateKey_file(" << key_file << ") error";
    return false;
  }

  if (SSL_CTX_check_private_key(m_ctx.get()) != 1) {
    DDG_LOG_DEBUG(g_logger)
        << "SSL_CTX_check_private_key cert_file =" << cert_file
        << " key_file = " << key_file;
    return false;
  }
  return true;
}

SSLSocket::ptr SSLSocket::CreateTCP(ddg::Address::ptr address) {
  SSLSocket::ptr sock(new SSLSocket(address->getFamily(), TCP, 0));
  return sock;
}

SSLSocket::ptr SSLSocket::CreateTCPSocket() {
  SSLSocket::ptr sock(new SSLSocket(IPv4, TCP, 0));
  return sock;
}

SSLSocket::ptr SSLSocket::CreateTCPSocket6() {
  SSLSocket::ptr sock(new SSLSocket(IPv6, TCP, 0));
  return sock;
}

std::ostream& SSLSocket::dump(std::ostream& os) const {
  os << "[SSLSocket sock=" << m_sock << " is_connected=" << m_isconnected
     << " family=" << m_family << " type=" << m_type
     << " protocol=" << m_protocol;
  if (m_local_address) {
    os << " local_address=" << m_local_address->toString();
  }
  if (m_remote_address) {
    os << " remote_address=" << m_remote_address->toString();
  }
  os << "]";
  return os;
}

namespace {

struct _SSLInit {
  _SSLInit() {
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
  }
};

static _SSLInit s_init;
}  // namespace

}  // namespace ddg
