#include "ddg/utils/memory_util.h"

#include <memory.h>

namespace ddg {

ScopedMalloc::ScopedMalloc(size_t size) noexcept : m_size(size) {
  m_vptr = malloc(m_size);
}

ScopedMalloc::ScopedMalloc(const ScopedMalloc& scopedmalloc) noexcept {
  m_size = scopedmalloc.m_size;
  m_vptr = malloc(m_size);
  memcpy(m_vptr, scopedmalloc.m_vptr, m_size);
}

ScopedMalloc::ScopedMalloc(ScopedMalloc&& scopedmalloc) noexcept {
  m_size = scopedmalloc.m_size;
  m_vptr = scopedmalloc.m_vptr;
  scopedmalloc.m_vptr = nullptr;
}

ScopedMalloc::~ScopedMalloc() {
  if (m_vptr) {
    free(m_vptr);
  }
}

void* ScopedMalloc::getRawPointer() const {
  return m_vptr;
}

}  // namespace ddg
