#pragma once

#include "rex_engine/gfx/system/command_allocator_pool.h"

#include "rex_engine/engine/types.h"
#include "rex_engine/pooling/growing_pool.h"
#include "rex_engine/engine/scoped_pooled_object.h"
#include "rex_engine/diagnostics/assert.h"

#include "rex_engine/gfx/system/command_allocator.h"
#include "rex_engine/gfx/core/graphics_engine_type.h"



namespace rex
{
  namespace gfx
  {
    // A structure combining an idle allocator and the fence value that's required
    // to have been reached to free up the allocator
    // When it's returned to the pool it's fence value is checked, making sure it's higher
    // than when originally allocated.
    class FencedAllocator
    {
    public:
      FencedAllocator(u64 fenceValue, rsl::unique_ptr<CommandAllocator> alloc);

      // Reset the fence value to a new value
      void reset_fence(u64 fenceValue);
      // Return the fence value
      u64 fence_value() const;

      // Easy access to underlying allocator object
      CommandAllocator* underlying_alloc();
      const CommandAllocator* underlying_alloc() const;

      CommandAllocator* operator->();
      const CommandAllocator* operator->() const;

    private:
      u64 m_fence_value; // The fence value required to be achieved in order to use the allocator
      rsl::unique_ptr<CommandAllocator> m_allocator;
    };

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