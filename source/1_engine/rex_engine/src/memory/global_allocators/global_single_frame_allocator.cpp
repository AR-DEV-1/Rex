#include "rex_engine/memory/global_allocators/global_single_frame_allocator.h"

#include "rex_engine/engine/engine.h"

// #TODO: Remaining cleanup of development/Pokemon -> main merge. ID: HEAP AND ALLOCATORS

namespace rex
{
  void* GlobalSingleFrameAllocator::allocate(const s32 count)
  {
    return engine::instance()->temp_alloc(count);
  }
  void GlobalSingleFrameAllocator::deallocate(void* const ptr, s32 /*count*/)
  {
    engine::instance()->temp_free(ptr);
  }

  s64 GlobalSingleFrameAllocator::max_size() const
  {
    return engine::instance()->temp_buffer_size();
  }

  bool GlobalSingleFrameAllocator::has_allocated_ptr(void* ptr) const
  {
    return engine::instance()->is_temp_alloc(ptr);
  }
}