#include "rex_engine/memory/global_allocators/global_scratch_allocator.h"

#include "rex_engine/engine/engine.h"

namespace rex
{
  void* GlobalScratchAllocator::allocate(const s32 count)
  {
    return engine::instance()->scratch_alloc(count);
  }
  void GlobalScratchAllocator::deallocate(void* const ptr, s32 /*count*/)
  {
    engine::instance()->scratch_free(ptr);
  }

  s32 GlobalScratchAllocator::max_size() const
  {
    return (rsl::numeric_limits<s32>::max)();
  }

  bool GlobalScratchAllocator::has_allocated_ptr(void* ptr) const
  {
    return engine::instance()->is_scratch_alloc(ptr);
  }
}