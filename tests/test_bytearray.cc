#include "ddg/bytearray.h"

#include "ddg/log.h"
#include "ddg/macro.h"

static ddg::Logger::ptr g_logger = DDG_LOG_ROOT();

int main() {
#define XX(type, len, write_fun, read_fun, base_len)                       \
  {                                                                        \
    std::vector<type> vec;                                                 \
    for (int i = 0; i < len; i++) {                                        \
      vec.push_back(static_cast<type>(rand() % len));                      \
    }                                                                      \
    ddg::ByteArray::ptr ba(new ddg::ByteArray(base_len));                  \
    for (auto& elem : vec) {                                               \
      ba->write_fun<type>(elem);                                           \
    }                                                                      \
    ba->setPosition(0);                                                    \
    for (size_t i = 0; i < vec.size(); ++i) {                              \
      type v = ba->read_fun<type>();                                       \
      if (v != vec[i]) {                                                   \
        DDG_LOG_DEBUG(g_logger) << "origin: " << vec[i] << " | "           \
                                << "after: " << v;                         \
      }                                                                    \
      DDG_ASSERT(v == vec[i]);                                             \
    }                                                                      \
    DDG_ASSERT(ba->getReadSize() == 0);                                    \
    DDG_LOG_INFO(g_logger) << #write_fun "/" #read_fun " (" #type ") len=" \
                           << len << " base_len=" << base_len              \
                           << " size=" << ba->getSize();                   \
  }

  XX(int8_t, 100, writeF, readF, 1);
  XX(uint8_t, 100, writeF, readF, 1);
  XX(int16_t, 100, writeF, readF, 1);
  XX(uint16_t, 100, writeF, readF, 1);
  XX(int32_t, 100, writeF, readF, 1);
  XX(uint32_t, 100, writeF, readF, 1);
  XX(int64_t, 100, writeF, readF, 1);
  XX(uint64_t, 100, writeF, readF, 1);
  XX(float, 100, writeF, readF, 1);
  XX(double, 100, writeF, readF, 1);

  XX(int8_t, 100, writeV, readV, 1);
  XX(uint8_t, 100, writeV, readV, 1);
  XX(int16_t, 100, writeV, readV, 1);
  XX(uint16_t, 100, writeV, readV, 1);
  XX(int32_t, 100, writeV, readV, 1);
  XX(uint32_t, 100, writeV, readV, 1);
  XX(int64_t, 100, writeV, readV, 1);
  XX(uint64_t, 100, writeV, readV, 1);
#undef XX

  return 0;
}
