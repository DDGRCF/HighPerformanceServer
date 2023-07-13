#include "ddg/tcp_server.h"

#include <memory>
#include <vector>

namespace ddg {
static ddg::Logger::ptr g_logger = DDG_LOG_ROOT();

static ddg::ConfigVar<time_t>::ptr g_tcp_server_read_timeout =
    ddg::Config::Lookup("tcp_server.read_timeout",
                        static_cast<time_t>(60 * 1000 * 2),
                        "tcp server read timeout");

bool TcpServerConf::isValid() const {
  return !address.empty();
}

bool TcpServerConf::operator==(const TcpServerConf& oth) const {
  return address == oth.address && keepalive == oth.keepalive &&
         timeout == oth.timeout && name == oth.name && ssl == oth.ssl &&
         cert_file == oth.cert_file && key_file == oth.key_file &&
         accept_worker == oth.accept_worker && io_worker == oth.io_worker &&
         process_worker == oth.process_worker && args == oth.args &&
         id == oth.id && type == oth.type;
}

TcpServer::TcpServer(ddg::IOManager* worker, ddg::IOManager* io_worker,
                     ddg::IOManager* accept_worker)
    : m_worker(worker),
      m_ioworker(io_worker),
      m_acceptworker(accept_worker),
      m_recvtimeout(g_tcp_server_read_timeout->getValue()),
      m_name("ddg/1.0.0"),
      m_isstop(true) {}

TcpServer::~TcpServer() {
  for (auto& sock : m_socks) {
    sock->close();
  }
  m_socks.clear();
}

void TcpServer::setConf(const TcpServerConf& v) {
  m_conf.reset(new TcpServerConf);
}

bool TcpServer::bind(ddg::Address::ptr addr, bool ssl) {
  std::vector<Address::ptr> addrs;
  std::vector<Address::ptr> fails;
  addrs.push_back(addr);
  return bind(addrs, fails, ssl);
}

// bind包括bind和监听，如果有失败的情况，就将失败的socket返回
bool TcpServer::bind(const std::vector<Address::ptr>& addrs,
                     std::vector<Address::ptr>& fails, bool ssl) {
  m_ssl = ssl;
  for (auto& addr : addrs) {
    Socket::ptr sock =
        ssl ? SSLSocket::CreateTcp(addr) : Socket::CreateTcp(addr);
    if (!sock->bind(addr)) {
      DDG_LOG_ERROR(g_logger)
          << "bind fail errno = " << errno << " errstr = " << strerror(errno)
          << " addr = " << *addr;
      fails.push_back(addr);
      continue;
    }
    if (!sock->listen()) {
      DDG_LOG_ERROR(g_logger)
          << " listen fail errno = " << errno << " errstr = " << strerror(errno)
          << " addr = " << *addr;
      fails.push_back(addr);
      continue;
    }
    m_socks.push_back(sock);
  }

  for (auto& sock : m_socks) {
    DDG_LOG_DEBUG(g_logger) << "type = " << m_type << " name = " << m_name
                            << " ssl = " << std::boolalpha << m_ssl
                            << " server bind success; " << *sock;
  }

  if (!fails.empty()) {
    return false;
  }

  return true;
}

// startAccept主要开始接收client，然后将client_socket加入到调度里面
void TcpServer::startAccept(Socket::ptr sock) {
  while (!m_isstop) {
    Socket::ptr client = sock->accept();
    if (client) {
      client->setRecvTimeout(m_recvtimeout);
      m_ioworker->schedule(
          std::bind(&TcpServer::handleClient, shared_from_this(), client));
    } else {
      DDG_LOG_ERROR(g_logger)
          << " accept errno = " << errno << " errstr = " << strerror(errno);
    }
  }
}

// 开始accept线程，接收每一个连接
bool TcpServer::start() {
  if (!m_isstop) {
    return true;
  }
  m_isstop = false;
  for (auto& sock : m_socks) {
    m_acceptworker->schedule(
        std::bind(&TcpServer::startAccept, shared_from_this(), sock));
  }
  return true;
}

void TcpServer::stop() {
  m_isstop = true;
  auto self = shared_from_this();
  m_acceptworker->schedule([self]() {
    for (auto& sock : self->m_socks) {
      sock->cancelAll();
      sock->close();
    }
    self->m_socks.clear();
  });
}

uint64_t TcpServer::getRecvTimeout() const {
  return m_recvtimeout;
}

std::string TcpServer::getName() const {
  return m_name;
}

void TcpServer::setRecvTimeout(uint64_t v) {
  m_recvtimeout = v;
}

void TcpServer::setName(const std::string& v) {
  m_name = v;
}

// 由于socket解引用后会自动close，因此连接上立马断开
void TcpServer::handleClient(Socket::ptr client) {
  DDG_LOG_DEBUG(g_logger) << "handleClient" << *client;
}

bool TcpServer::loadCertificates(const std::string& cert_file,
                                 const std::string& key_file) {  // TODO:
  for (auto& sock : m_socks) {
    auto ssl_socket = std::dynamic_pointer_cast<SSLSocket>(sock);
    if (ssl_socket) {
      if (!ssl_socket->loadCertificates(cert_file, key_file)) {
        return false;
      }
    }
  }
  return true;
}

std::string TcpServer::toString(const std::string& prefix) {
  std::stringstream ss;
  ss << prefix << "[itype=" << m_type << " name = " << m_name
     << " ssl = " << std::boolalpha << m_ssl
     << " worker = " << (m_worker ? m_worker->getName() : "none")
     << " accept = " << (m_acceptworker ? m_acceptworker->getName() : "none")
     << std::endl;
  std::string pfx = prefix.empty() ? "    " : prefix;
  for (auto& sock : m_socks) {
    ss << pfx << pfx << *sock << std::endl;
  }
  return ss.str();
}

}  // namespace ddg
