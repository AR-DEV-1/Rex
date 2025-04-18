#include "rex_engine/gfx/system/command_allocator_pool.h"

#include "rex_engine/engine/types.h"
#include "rex_engine/diagnostics/assert.h"

#include "rex_engine/gfx/system/command_allocator.h"

#include "rex_engine/gfx/graphics.h"

#include "rex_engine/containers/vector_utils.h"



namespace rex
{
  namespace gfx
  {
    FencedAllocator::FencedAllocator(u64 fenceValue, rsl::unique_ptr<CommandAllocator> alloc)
      : m_fence_value(fenceValue)
      , m_allocator(rsl::move(alloc))
    {

    }

    void FencedAllocator::reset_fence(u64 fenceValue)
    {
      m_fence_value = fenceValue;
    }
    u64 FencedAllocator::fence_value() const
    {
      return m_fence_value;
    }

    CommandAllocator* FencedAllocator::underlying_alloc()
    {
      return m_allocator.get();
    }
    const CommandAllocator* FencedAllocator::underlying_alloc() const
    {
      return m_allocator.get();
    }
    CommandAllocator* FencedAllocator::operator->()
    {
      return m_allocator.get();
    }
    const CommandAllocator* FencedAllocator::operator->() const
    {
      return m_allocator.get();
    }

    ScopedFencedAllocator::ScopedFencedAllocator()
      : ScopedFencedAllocator(static_cast<u64>(-1), nullptr, nullptr)
    {}

    ScopedFencedAllocator::ScopedFencedAllocator(u64 initialFenceValue, FencedAllocator* alloc, GrowingPool<FencedAllocator>* pool)
      : ScopedPoolObject<FencedAllocator>(alloc, pool)
      , m_initial_fence_value(initialFenceValue)
    {}
    ScopedFencedAllocator::~ScopedFencedAllocator()
    {
      if (get())
      {
        REX_ASSERT_X(m_initial_fence_value < get()->fence_value(), "Make sure you increase an allocator's fence value before you return it back to the pool");
      }
    }

    CommandAllocatorPool::CommandAllocatorPool(GraphicsEngineType type)
      : m_type(type)
      , m_pool()
    {}

    // Request a new allocator from the pool, create a new one if one isn't found
    ScopedFencedAllocator CommandAllocatorPool::request_allocator(u64 fenceValue)
    {
      // Find an allocator who's fence value is equal or higher than the given fence value
      FencedAllocator* alloc = m_pool.request(
        [fenceValue](const rsl::unique_ptr<FencedAllocator>& alloc) // find func
        {
          return alloc->fence_value() <= fenceValue;
        },
        [this]() // create func
        { 
          return rsl::make_unique<FencedAllocator>(0, gfx::gal::instance()->create_command_allocator(m_type)); 
        }
      );

      return ScopedFencedAllocator(fenceValue, alloc, &m_pool);
    }
  }
}