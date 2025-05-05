#include "rex_engine/memory/global_allocators/global_scratch_allocator.h"

#include "rex_engine/engine/engine.h"

namespace rex
{
  void* GlobalScratchAllocator::allocate(const s64 count)
  {
    return engine::instance()->scratch_alloc(count);
  }
  void* GlobalScratchAllocator::reallocate(void* ptr, s64 count)
  {
    return engine::instance()->scratch_realloc(ptr, count);
  }
  void GlobalScratchAllocator::deallocate(void* const ptr, s64 /*count*/)
  {
    engine::instance()->scratch_free(ptr);
  }

  s64 GlobalScratchAllocator::max_size() const
  {
    return engine::instance()->scratch_buffer_size();
  }

  bool GlobalScratchAllocator::has_allocated_ptr(void* ptr) const
  {
    return engine::instance()->is_scratch_alloc(ptr);
  }
}