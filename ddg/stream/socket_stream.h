#ifndef DDG_SOCKET_STREAM_H_
#define DDG_SOCKET_STREAM_H_

#include "ddg/iomanager.h"
#include "ddg/mutex.h"
#include "ddg/noncopyable.h"
#include "ddg/socket.h"
#include "ddg/stream.h"

namespace ddg {

// 对Stream封装
class SocketStream : public Stream, public NonCopyable {
 public:
  using ptr = std::shared_ptr<SocketStream>;

  SocketStream(Socket::ptr sock, bool owner = true);

  ~SocketStream();

  virtual int read(void* buffer, size_t length) override;

  virtual int read(ByteArray::ptr ba, size_t length) override;

  virtual int write(const void* buffer, size_t length) override;

  virtual int write(ByteArray::ptr ba, size_t length) override;

  virtual void close() override;

  Socket::ptr getSocket() const { return m_socket; }

  bool isConnected() const;

  Address::ptr getRemoteAddress();

  Address::ptr getLocalAddress();

  std::string getRemoteAddressString();

  std::string getLocalAddressString();

 protected:
  /// Socket类
  Socket::ptr m_socket;
  /// 是否主控
  bool m_owner;
};

}  // namespace ddg

#endif
