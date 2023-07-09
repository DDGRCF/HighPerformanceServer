#ifndef DDG_ENDIAN_H_
#define DDG_ENDIAN_H_

#include <byteswap.h>
#include <stdint.h>
#include <type_traits>

#define DDG_LITTLE_ENDIAN 1
#define DDG_BIG_ENDIAN 2

namespace ddg {

template <class T>
typename std::enable_if<sizeof(T) == sizeof(uint64_t), T>::type byteswap(
    T value) {
  return static_cast<T>(bswap_64(static_cast<uint64_t>(value)));
}

template <class T>
typename std::enable_if<sizeof(T) == sizeof(uint32_t), T>::type byteswap(
    T value) {
  return static_cast<T>(bswap_32(static_cast<uint64_t>(value)));
}

template <class T>
typename std::enable_if<sizeof(T) == sizeof(uint16_t), T>::type byteswap(
    T value) {
  return static_cast<T>(bswap_16(static_cast<uint16_t>(value)));
}

#if BYTE_ORDER == BIG_ENDIAN
#define DDG_BYTE_ORDER DDG_BIG_ENDIAN
#else
#define DDG_BYTE_ORDER DDG_LITTLE_ENDIAN
#endif

#if DDG_BYTE_ORDER == DDG_BIG_ENDIAN

template <class T>
T byteswapOnLittleEndian(T t) {
  return t;
}

template <class T>
T byteswapOnBigEndian(T t) {
  return byteswap(t);
}
#else

template <class T>
T byteswapOnLittleEndian(T t) {
  return byteswap(t);
}

template <class T>
T byteswapOnBigEndian(T t) {
  return t;
}
#endif

}  // namespace ddg

#endif
