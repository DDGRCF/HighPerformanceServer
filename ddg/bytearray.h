#ifndef DDG_BYTEARRAY_H_
#define DDG_BYTEARRAY_H_

#include <memory.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <memory>
#include <string>
#include <vector>

#include "ddg/endian.h"

namespace ddg {

class ByteArray {
 public:
  using ptr = std::shared_ptr<ByteArray>;

  struct Node {
    Node();

    Node(size_t s);

    ~Node();

    char* ptr;

    Node* next;

    size_t size;
  };

 private:
  static uint16_t EncodeZigzag16(int16_t v) {
    return static_cast<uint16_t>((v << 1) ^ (v >> 15));
  }

  static uint32_t EncodeZigzag32(int32_t v) {
    return static_cast<uint32_t>((v << 1) ^ (v >> 31));
  }

  static uint64_t EncodeZigzag64(
      int64_t v) {  // TODOexplicit specialization in non-namespace scope: test
    return static_cast<uint64_t>((v << 1 ^ (v >> 63)));
  }

  static int16_t DecodeZigzag16(uint16_t v) {
    return static_cast<int16_t>((v >> 1) ^ -(v & 1));
  }

  static int32_t DecodeZigzag32(uint32_t v) {
    return static_cast<int32_t>((v >> 1) ^ -(v & 1));
  }

  static int64_t DecodeZigzag64(uint64_t v) {
    return static_cast<int64_t>((v >> 1) ^ -(v & 1));
  }

 public:
  ByteArray(size_t base_size = 4096);

  ~ByteArray();

 public:
  bool isLittleEndian() const;

  void setIsLittleEndian(bool val);

  bool empty() const;

 private:
  size_t getCapacity() const;

  void addCapacity(size_t size);

 public:
  std::string toString() const;

  std::string toHexString() const;

  size_t getReadSize() const;

  size_t getSize() const;

  void setPosition(size_t v);

  size_t getPosition() const;

 public:
  void write(const void* buf, size_t size);

  void read(void* buf, size_t size);

  void read(void* buf, size_t size, size_t position) const;

  void clear();

 public:
  // writeF
  template <class T>
  void writeF(T value) {
    if (m_endian != DDG_BYTE_ORDER) {
      value = byteswap<T>(value);
    }
    write(&value, sizeof(value));
  }

  // writeV @brief 每7位存储有数据，前面的0表示前面没有数据，前面的1表示前面还有数据
  template <class T>
  void writeV(T value) {
    uint8_t tmp[sizeof(T) * 8 / 7 + 1];
    uint8_t i = 0;
    while (value >= 0x80) {
      tmp[i++] = (value & 0x7F) | 0x80;
      value >>= 7;
    }
    tmp[i++] = value;
    write(tmp, i);
  }

  // writeS
  template <class T>
  void writeSF(const std::string& value) {
    writeF<T>(static_cast<T>(value.size()));
    write(value.c_str(), value.size());
  }

  template <class T>
  void writeSV(const std::string& value) {
    writeV<T>(static_cast<T>(value.size()));
    write(value.c_str(), value.size());
  }

  void writeS(const std::string& value) { write(value.c_str(), value.size()); }

  // readF 固定长度
  template <class T>
  T readF() {
    T v;
    read(&v, sizeof(v));
    if (m_endian != DDG_BYTE_ORDER) {
      return byteswap<T>(v);
    }
    return v;
  }

  // readV 变长
  template <class T>
  T readV() {
    uint32_t result = 0;
    for (int i = 0; i < static_cast<int>(sizeof(T)) * 8; i += 7) {
      uint8_t b = readF<uint8_t>();
      if (b < 0x80) {
        result |= (static_cast<T>(b) << i);
        break;
      } else {
        result |= (static_cast<T>(b & 0x7F) << i);
      }
    }
    return result;
  }

  // readSF
  template <class T>
  std::string readSF() {
    T len = readF<T>();
    std::string buf;
    buf.resize(len);
    read(&buf[0], len);
    return buf;
  }

  // readSV
  template <class T>
  std::string readSV() {
    T len = readV<T>();
    std::string buf;
    buf.resize(len);
    read(&buf, len);
    return buf;
  }

  // readS
  std::string readS(size_t len) {
    std::string buf;
    buf.resize(len);
    read(&buf, len);
    return buf;
  }

 public:
  bool writeToFile(const std::string& name) const;

  bool readFromFile(const std::string& name);

  uint64_t getReadBuffers(std::vector<iovec>& buffers, uint64_t len) const;

  uint64_t getReadBuffers(std::vector<iovec>& buffers, uint64_t len,
                          uint64_t position) const;

  uint64_t getWriteBuffers(std::vector<iovec>& buffers, uint64_t len);

 private:
  size_t m_basesize;
  size_t m_position;
  size_t m_capacity;
  size_t m_size;
  int8_t m_endian;

  Node* m_root;
  Node* m_cur;
};

template <>
void ByteArray::writeF<int8_t>(int8_t value);

template <>
void ByteArray::writeF<uint8_t>(uint8_t value);

template <>
void ByteArray::writeF<float>(float value);

template <>
void ByteArray::writeF<double>(double value);

template <>
void ByteArray::writeV<int8_t>(int8_t value);

template <>
void ByteArray::writeV<uint8_t>(uint8_t value);

template <>
void ByteArray::writeV<int16_t>(int16_t value);

template <>
void ByteArray::writeV<int32_t>(int32_t value);

template <>
void ByteArray::writeV<int64_t>(int64_t value);

template <>
int8_t ByteArray::readF<int8_t>();

template <>
uint8_t ByteArray::readF<uint8_t>();

template <>
float ByteArray::readF<float>();

template <>
double ByteArray::readF<double>();

template <>
int8_t ByteArray::readV<int8_t>();

template <>
uint8_t ByteArray::readV<uint8_t>();

template <>
int16_t ByteArray::readV<int16_t>();

template <>
int32_t ByteArray::readV<int32_t>();

template <>
int64_t ByteArray::readV<int64_t>();

}  // namespace ddg

#endif
