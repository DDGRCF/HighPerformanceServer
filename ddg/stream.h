#ifndef DDG_STREAM_H_
#define DDG_STREAM_H_

#include <stdint.h>
#include <memory>

#include <ddg/bytearray.h>

namespace ddg {
class Stream {
 public:
  using ptr = std::shared_ptr<Stream>;

  virtual int read(void* buffer, size_t length) = 0;
  virtual int read(ByteArray::ptr ba, size_t length) = 0;

  virtual int readFixSize(void* buffer, size_t length);
  virtual int readFixSize(ByteArray::ptr ba, size_t length);

  virtual int write(const void* buffer, size_t length) = 0;
  virtual int write(ByteArray::ptr ba, size_t length) = 0;

  virtual int writeFixSize(const void* buffer, size_t length);
  virtual int writeFixSize(const ByteArray::ptr bs, size_t length);

  virtual void close() = 0;
};

}  // namespace ddg

#endif
