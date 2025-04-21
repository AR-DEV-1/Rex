#pragma once

#include "rex_engine/engine/types.h"
#include "rex_engine/pooling/growing_pool.h"
#include "rex_engine/pooling/scoped_pooled_object.h"

#include "rex_engine/gfx/system/command_allocator.h"
#include "rex_engine/gfx/core/graphics_engine_type.h"

namespace rex
{
  namespace gfx
  {
    // An fenced allocator that automatically returns to the pool it came from on destruction
    // An assert is present in the dtor to make sure that the fence got incremented before it returns to the pool
    class ScopedFencedAllocator : public ScopedPoolObject<FencedAllocator>
    {
    public:
      ScopedFencedAllocator();
      ScopedFencedAllocator(u64 initialFenceValue, FencedAllocator* alloc, GrowingPool<FencedAllocator>* pool);
      ScopedFencedAllocator(const ScopedFencedAllocator&) = delete;
      ScopedFencedAllocator(ScopedFencedAllocator&&) = default;
      ~ScopedFencedAllocator();

      ScopedFencedAllocator& operator=(const ScopedFencedAllocator&) = delete;
      ScopedFencedAllocator& operator=(ScopedFencedAllocator&&) = default;

    private:
      u64 m_initial_fence_value;
    };

    // The pool holding all command allocators
    class CommandAllocatorPool
    {
    public:
      CommandAllocatorPool(GraphicsEngineType type);

      // Request a new allocator from the pool, create a new one if one isn't found
      ScopedFencedAllocator request_allocator(u64 fenceValue);

    private:
      GrowingPool<FencedAllocator> m_pool;
      GraphicsEngineType m_type;
    };
  }
}