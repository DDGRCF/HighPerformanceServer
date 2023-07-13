#ifndef DDG_UTILS_MEMORY_UTIL_H__
#define DDG_UTILS_MEMORY_UTIL_H__

#include <stdlib.h>

namespace ddg {

class ScopedMalloc {
 public:
  explicit ScopedMalloc(size_t size) noexcept;

  ScopedMalloc(const ScopedMalloc& scopedmalloc) noexcept;

  ScopedMalloc(ScopedMalloc&& scopedmalloc) noexcept;

  ~ScopedMalloc();

  template <class T>
  T getPointer() const {
    return static_cast<T>(m_vptr);
  }

  void* getRawPointer() const;

 private:
  void* m_vptr = nullptr;
  size_t m_size;
};

}  // namespace ddg

#endif
