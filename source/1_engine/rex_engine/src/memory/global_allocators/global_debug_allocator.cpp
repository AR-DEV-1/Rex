#include "rex_engine/memory/global_allocators/global_debug_allocator.h"

#include "rex_engine/engine/globals.h"

namespace rex
{
  void* GlobalDebugAllocator::allocate(const s64 count)
  {
    return m_alloc.allocate(count);
  }
  void GlobalDebugAllocator::deallocate(void* const ptr, s64 /*count*/)
  {
    return m_alloc.deallocate(ptr);
  }

  s64 GlobalDebugAllocator::max_size() const
  {
    return (rsl::numeric_limits<s64>::max)();
  }

} // namespace rex