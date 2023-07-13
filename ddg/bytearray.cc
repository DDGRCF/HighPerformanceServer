#include "ddg/bytearray.h"

#include <cmath>
#include <iomanip>

#include "ddg/endian.h"
#include "ddg/log.h"

namespace ddg {

static Logger::ptr g_logger = DDG_LOG_ROOT();

ByteArray::Node::Node(size_t s) : ptr(new char[s]), next(nullptr), size(s) {}

ByteArray::Node::Node() : ptr(nullptr), next(nullptr), size(0) {}

ByteArray::Node::~Node() {
  if (ptr) {
    delete[] ptr;
  }
}

uint16_t ByteArray::EncodeZigzag16(int16_t v) {
  return static_cast<uint16_t>((v << 1) ^ (v >> 15));
}

uint32_t ByteArray::EncodeZigzag32(int32_t v) {
  return static_cast<uint32_t>((v << 1) ^ (v >> 31));
}

uint64_t ByteArray::EncodeZigzag64(int64_t v) {
  return static_cast<uint64_t>((v << 1 ^ (v >> 63)));
}

int16_t ByteArray::DecodeZigzag16(uint16_t v) {
  return static_cast<int16_t>((v >> 1) ^ -(v & 1));
}

int32_t ByteArray::DecodeZigzag32(uint32_t v) {
  return static_cast<int32_t>((v >> 1) ^ -(v & 1));
}

int64_t ByteArray::DecodeZigzag64(uint64_t v) {
  return static_cast<int64_t>((v >> 1) ^ -(v & 1));
}

ByteArray::ByteArray(size_t base_size)
    : m_basesize(base_size),
      m_position(0),
      m_capacity(base_size),
      m_size(0),
      m_endian(DDG_BIG_ENDIAN),
      m_root(new Node(base_size)) {
  m_cur = m_root;
}

ByteArray::~ByteArray() {
  Node* node = m_root;
  while (node) {
    m_cur = node;
    node = node->next;
    delete m_cur;
  }
}

bool ByteArray::isLittleEndian() const {
  return m_endian == DDG_LITTLE_ENDIAN;
}

void ByteArray::setIsLittleEndian(bool val) {
  if (val) {
    m_endian = DDG_LITTLE_ENDIAN;
  } else {
    m_endian = DDG_BIG_ENDIAN;
  }
}

bool ByteArray::empty() const {
  return m_size == 0;
}

// 对外容量 = 原始容量 - 读写指针位置
size_t ByteArray::getCapacity() const {
  return m_capacity - m_position;
}

// 添加容量，读写指针不变
void ByteArray::addCapacity(size_t size) {
  if (size == 0) {
    return;
  }

  size_t old_cap = getCapacity();
  if (old_cap >= size) {
    return;
  }

  size = size - old_cap;
  size_t count = std::ceil(static_cast<double>(size) / m_basesize);
  Node* node = m_root;

  while (node->next) {
    node = node->next;
  }

  Node* first = nullptr;
  for (size_t i = 0; i < count; i++) {
    node->next = new Node(m_basesize);
    if (first == nullptr) {
      first = node->next;
    }
    node = node->next;
    m_capacity += m_basesize;
  }

  if (old_cap == 0) {
    m_cur = first;
  }
}

// 获取可读的数据大小
size_t ByteArray::getReadSize() const {
  return m_size - m_position;
}

// 写数据，写完后指针会变动
void ByteArray::write(const void* buf, size_t size) {
  if (size == 0) {
    return;
  }

  addCapacity(size);

  size_t npos = m_position % m_basesize;
  size_t ncap = m_cur->size - npos;
  size_t bpos = 0;

  while (size > 0) {
    if (ncap >= size) {
      memcpy(m_cur->ptr + npos, static_cast<const char*>(buf) + bpos, ncap);
      m_position += size;
      bpos += size;
      size = 0;
    } else {
      memcpy(m_cur->ptr + npos, static_cast<const char*>(buf) + bpos, ncap);
      m_position += ncap;
      bpos += ncap;
      size -= ncap;
      m_cur = m_cur->next;
      ncap = m_cur->size;
      npos = 0;
    }
  }

  if (m_position > m_size) {
    m_size = m_position;
  }
}

// 读数据，读完后指针会变动
void ByteArray::read(void* buf, size_t size) {
  if (size > getReadSize()) {
    throw std::out_of_range("not enough len");
  }

  size_t npos = m_position % m_basesize;
  size_t ncap = m_cur->size - npos;
  size_t bpos = 0;
  while (size > 0) {
    if (ncap >= size) {
      memcpy(static_cast<char*>(buf) + bpos, m_cur->ptr + npos, size);
      if (m_cur->size == npos + size) {
        m_cur = m_cur->next;
      }
      m_position += size;
      bpos += size;
      size = 0;
    } else {
      memcpy(static_cast<char*>(buf) + bpos, m_cur->ptr + npos, ncap);
      m_position += ncap;
      bpos += ncap;
      size -= ncap;
      m_cur = m_cur->next;
      ncap = m_cur->size;
      npos = 0;
    }
  }
}

// 从指定位置开始读数据，这个过程内部指针是不变动的
void ByteArray::read(void* buf, size_t size, size_t position) const {
  if (size > getReadSize()) {
    throw std::out_of_range("not enough len");
  }

  size_t npos = position % m_basesize;
  size_t ncap = m_cur->size - npos;
  size_t bpos = 0;

  Node* node = m_cur;
  while (size > 0) {
    if (ncap >= size) {
      memcpy(static_cast<char*>(buf) + bpos, node->ptr + npos, size);
      if (node->size == npos + size) {
        node = node->next;
      }
      position += size;
      bpos += size;
      size = 0;
    } else {
      memcpy(static_cast<char*>(buf) + bpos, node->ptr + npos, ncap);
      position += ncap;
      bpos += ncap;
      size -= ncap;
      node = node->next;
      ncap = node->size;
      npos = 0;
    }
  }
}

// 设置指针位置
void ByteArray::setPosition(size_t v) {
  if (v > m_capacity) {
    throw std::out_of_range("set_position out of range");
  }
  m_position = v;
  if (m_position > m_size) {
    m_size = m_position;
  }
  m_cur = m_root;
  while (v > m_cur->size) {
    v -= m_cur->size;
    m_cur = m_cur->next;
  }

  if (v == m_cur->size) {
    m_cur = m_cur->next;
  }
}

// 获取指针位置
size_t ByteArray::getPosition() const {
  return m_position;
}

// 获得数据大小
size_t ByteArray::getSize() const {
  return m_size;
}

// 清除内容，指针变动
void ByteArray::clear() {
  m_position = m_size = 0;
  m_capacity = m_basesize;
  Node* tmp = m_root->next;
  while (tmp) {
    m_cur = tmp;
    tmp = tmp->next;
    delete m_cur;
  }
  m_cur = m_root;
  m_root->next = nullptr;
}

// 从当前位置的指针开始，获取后的所有字符串，期间不移动指针
std::string ByteArray::toString() const {
  std::string str;
  str.resize(getReadSize());
  if (str.empty()) {
    return str;
  }
  read(&str[0], str.size(), m_position);
  return str;
}

// 转为为16进制
std::string ByteArray::toHexString() const {
  std::string str = toString();
  std::stringstream ss;

  for (size_t i = 0; i < str.size(); ++i) {
    if (i > 0 && i % 32 == 0) {
      ss << std::endl;
    }
    ss << std::setw(2) << std::setfill('0') << std::hex << (int)(uint8_t)str[i]
       << " ";
  }

  return ss.str();
}

// 将数据写进入文件中，指针不变动
bool ByteArray::writeToFile(const std::string& name) const {
  std::ofstream ofs;
  ofs.open(name, std::ios::trunc | std::ios::binary);
  if (!ofs) {
    DDG_LOG_DEBUG(g_logger)
        << "ByteArray::writeToFile name = " << name
        << " error, errno = " << errno << " errstr = " << strerror(errno);
    return false;
  }

  int64_t read_size = getReadSize();
  int64_t pos = m_position;

  Node* node = m_cur;

  while (read_size > 0) {
    int diff = pos % m_basesize;
    int64_t len = (read_size > static_cast<int64_t>(m_basesize) ? m_basesize
                                                                : read_size) -
                  diff;
    ofs.write(node->ptr + diff, len);
    node = node->next;
    pos += len;
    read_size -= len;
  }

  return true;
}

// 将数据从文件中读出来，指针变动
bool ByteArray::readFromFile(const std::string& name) {
  std::ifstream ifs;
  ifs.open(name, std::ios::binary);
  if (!ifs) {
    DDG_LOG_DEBUG(g_logger)
        << "readFrom name = " << name << " error, errno = " << errno
        << " errstr = " << strerror(errno);
    return false;
  }
  std::shared_ptr<char> buf(new char[m_basesize],
                            [](char* ptr) { delete[] ptr; });
  while (!ifs.eof()) {
    ifs.read(buf.get(), m_basesize);
    write(buf.get(), ifs.gcount());
  }
  return true;
}

// 获得可读的buffer，指针不变动
uint64_t ByteArray::getReadBuffers(std::vector<iovec>& buffers,
                                   uint64_t len) const {
  len = len > getReadSize() ? getReadSize() : len;
  if (len == 0) {
    return 0;
  }

  uint64_t size = len;

  size_t npos = m_position % m_basesize;

  size_t ncap = m_cur->size - npos;

  struct iovec iov;
  Node* node = m_cur;

  while (len > 0) {
    if (ncap >= len) {
      iov.iov_base = node->ptr + npos;
      iov.iov_len = len;
      len = 0;
    } else {
      iov.iov_base = node->ptr + npos;
      iov.iov_len = ncap;
      len -= ncap;
      node = node->next;
      ncap = node->size;
      npos = 0;
    }
    buffers.push_back(iov);
  }
  return size;
}

// 从当前位置获得可写的buffer
uint64_t ByteArray::getReadBuffers(std::vector<iovec>& buffers, uint64_t len,
                                   uint64_t position) const {
  len = len > getReadSize() ? getReadSize() : len;
  if (len == 0) {
    return 0;
  }

  uint64_t size = len;

  size_t npos = position % m_basesize;
  size_t count = position / m_basesize;
  Node* cur = m_root;
  while (count > 0) {
    cur = cur->next;
    --count;
  }

  size_t ncap = cur->size - npos;
  struct iovec iov;
  while (len > 0) {
    if (ncap >= len) {
      iov.iov_base = cur->ptr + npos;
      iov.iov_len = len;
      len = 0;
    } else {
      iov.iov_base = cur->ptr + npos;
      iov.iov_len = ncap;
      len -= ncap;
      cur = cur->next;
      ncap = cur->size;
      npos = 0;
    }
    buffers.push_back(iov);
  }
  return size;
}

// 获得可以写的buffer
uint64_t ByteArray::getWriteBuffers(std::vector<iovec>& buffers, uint64_t len) {
  if (len == 0) {
    return 0;
  }
  addCapacity(len);
  uint64_t size = len;

  size_t npos = m_position % m_basesize;
  size_t ncap = m_cur->size - npos;
  struct iovec iov;
  Node* cur = m_cur;
  while (len > 0) {
    if (ncap >= len) {
      iov.iov_base = cur->ptr + npos;
      iov.iov_len = len;
      len = 0;
    } else {
      iov.iov_base = cur->ptr + npos;
      iov.iov_len = ncap;

      len -= ncap;
      cur = cur->next;
      ncap = cur->size;
      npos = 0;
    }
    buffers.push_back(iov);
  }
  return size;
}

template <>
void ByteArray::writeF<int8_t>(int8_t value) {
  write(&value, sizeof(value));
}

template <>
void ByteArray::writeF<uint8_t>(uint8_t value) {
  write(&value, sizeof(value));
}

template <>
void ByteArray::writeF<float>(float value) {
  uint32_t v;
  memcpy(&v, &value, sizeof(value));
  writeF<uint32_t>(v);
}

template <>
void ByteArray::writeF<double>(double value) {
  uint64_t v;
  memcpy(&v, &value, sizeof(value));
  writeF<uint64_t>(v);
}

template <>
void ByteArray::writeV<int8_t>(int8_t value) {
  writeF<int8_t>(value);
}

template <>
void ByteArray::writeV<uint8_t>(uint8_t value) {
  writeF<uint8_t>(value);
}

template <>
void ByteArray::writeV<int16_t>(int16_t value) {
  writeV<uint16_t>(EncodeZigzag16(value));
}

template <>
void ByteArray::writeV<int32_t>(int32_t value) {
  writeV<uint32_t>(EncodeZigzag32(value));
}

template <>
void ByteArray::writeV<int64_t>(int64_t value) {
  writeV<uint64_t>(EncodeZigzag64(value));
}

template <>
int8_t ByteArray::readF<int8_t>() {
  int8_t v;
  read(&v, sizeof(v));
  return v;
}

template <>
uint8_t ByteArray::readF<uint8_t>() {
  uint8_t v;
  read(&v, sizeof(v));
  return v;
}

template <>
float ByteArray::readF<float>() {
  uint32_t v = readF<uint32_t>();
  float value;
  memcpy(&value, &v, sizeof(v));
  return value;
}

template <>
double ByteArray::readF<double>() {
  uint64_t v = readF<uint64_t>();
  double value;
  memcpy(&value, &v, sizeof(v));
  return value;
}

template <>
int8_t ByteArray::readV<int8_t>() {
  return readF<int8_t>();
}

template <>
uint8_t ByteArray::readV<uint8_t>() {
  return readF<uint8_t>();
}

template <>
int16_t ByteArray::readV<int16_t>() {
  return DecodeZigzag16(readV<uint16_t>());
}

template <>
int32_t ByteArray::readV<int32_t>() {
  return DecodeZigzag32(readV<uint32_t>());
}

template <>
int64_t ByteArray::readV<int64_t>() {
  return DecodeZigzag64(readV<uint64_t>());
}

}  // namespace ddg
