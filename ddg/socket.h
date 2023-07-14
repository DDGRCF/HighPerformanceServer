#ifndef DDG_SOCKET_H_
#define DDG_SOCKET_H_

#include <memory>

#include <arpa/inet.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

#include "ddg/address.h"
#include "ddg/noncopyable.h"

namespace ddg {

class Socket : public std::enable_shared_from_this<Socket>, NonCopyable {

 public:
  using ptr = std::shared_ptr<Socket>;
  using weak_ptr = std::weak_ptr<Socket>;

  // TODO: 字符串转化
  enum Type {
    TCP = SOCK_STREAM,
    UDP = SOCK_DGRAM,
  };

  enum Family {
    IPv4 = AF_INET,
    IPv6 = AF_INET6,
    UNIX = AF_UNIX,
  };

 public:
  Socket(int family, int type, int protocol = 0);

  virtual ~Socket();

  time_t getSendTimeout();

  void setSendTimeout(time_t v);

  time_t getRecvTimeout();

  void setRecvTimeout(time_t v);

  bool getOption(int level, int option, void* result, socklen_t* len) const;

  template <class T>
  bool getOption(int level, int option, T& result) const {
    socklen_t length = sizeof(T);
    return getOption(level, option, &result, &length);
  }

  bool setOption(int level, int option, const void* result, socklen_t len);

  template <class T>
  bool setOption(int level, int option, const T& value) {
    return setOption(level, option, &value, sizeof(T));
  }

  virtual Socket::ptr accept();

  virtual bool bind(const Address::ptr addr);

  virtual bool connect(const Address::ptr addr, time_t timeout_ms = -1);

  virtual bool reconnect(time_t timeout_ms = -1);

  virtual bool listen(int backlog = SOMAXCONN);

  virtual bool close();

  virtual int send(const void* buffer, size_t length, int flags = 0);

  virtual int send(const iovec* buffers, size_t length, int flags = 0);

  virtual int sendTo(const void* buffer, size_t length, const Address::ptr to,
                     int flags = 0);

  virtual int sendTo(const iovec* buffers, size_t length, const Address::ptr to,
                     int flags = 0);

  virtual int recv(void* buffer, size_t length, int flags = 0);

  virtual int recv(iovec* buffers, size_t length, int flags = 0);

  virtual int recvFrom(void* buffer, size_t length, Address::ptr from,
                       int flags = 0);

  virtual int recvFrom(iovec* buffer, size_t length, Address::ptr from,
                       int flags = 0);

 public:
  Address::ptr getRemoteAddress();

  Address::ptr getLocalAddress();

  int getFamily() const;

  int getType() const;

  int getProtocal() const;

  bool isConnected() const;

  bool isValid() const;

  int getError() const;

  virtual std::ostream& dump(std::ostream& os) const;

  virtual std::string toString() const;

  friend std::ostream& operator<<(std::ostream& os, const Socket& sock);

  int getSocket() const;

  bool cancelRead();

  bool cancelWrite();

  bool cancelAccept();

  bool cancelAll();

 public:
  static Socket::ptr CreateTcp(ddg::Address::ptr address);

  static Socket::ptr CreateUdp(ddg::Address::ptr address);

  static Socket::ptr CreateTcpSocket();

  static Socket::ptr CreateUdpSocket();

  static Socket::ptr CreateTcpSocket6();

  static Socket::ptr CreateUdpSocket6();

  static Socket::ptr CreateUnixTcpSocket();

  static Socket::ptr CreateUnixUdpSocket();

 protected:
  void initSock();

  void newSock();

  virtual bool init(int sock);

 protected:
  int m_sock;

  int m_family;

  int m_type;

  int m_protocol;

  bool m_isconnected;

  Address::ptr m_local_address;

  Address::ptr m_remote_address;
};

class SSLSocket : public Socket {
 public:
  using ptr = std::shared_ptr<SSLSocket>;

  static SSLSocket::ptr CreateTcp(ddg::Address::ptr address);
  static SSLSocket::ptr CreateTCPSocket();
  static SSLSocket::ptr CreateTCPSocket6();

  SSLSocket(int family, int type, int protocol = 0);
  virtual Socket::ptr accept() override;
  virtual bool bind(const Address::ptr addr) override;
  virtual bool connect(const Address::ptr addr,
                       time_t timeout_ms = -1) override;
  virtual bool listen(int backlog = SOMAXCONN) override;
  virtual bool close() override;
  virtual int send(const void* buffer, size_t length, int flags = 0) override;
  virtual int send(const iovec* buffers, size_t length, int flags = 0) override;
  virtual int sendTo(const void* buffer, size_t length, const Address::ptr to,
                     int flags = 0) override;
  virtual int sendTo(const iovec* buffers, size_t length, const Address::ptr to,
                     int flags = 0) override;
  virtual int recv(void* buffer, size_t length, int flags = 0) override;
  virtual int recv(iovec* buffers, size_t length, int flags = 0) override;
  virtual int recvFrom(void* buffer, size_t length, Address::ptr from,
                       int flags = 0) override;
  virtual int recvFrom(iovec* buffers, size_t length, Address::ptr from,
                       int flags = 0) override;

  bool loadCertificates(const std::string& cert_file,
                        const std::string& key_file);
  virtual std::ostream& dump(std::ostream& os) const override;

 protected:
  virtual bool init(int sock) override;

 private:
  std::shared_ptr<SSL_CTX> m_ctx;
  std::shared_ptr<SSL> m_ssl;
};

}  // namespace ddg

#endif
