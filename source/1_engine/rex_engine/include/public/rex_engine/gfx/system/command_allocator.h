#pragma once

namespace rex
{
  namespace gfx
  {
    // A base interface for an allocator used for queueing gpu commands
    class CommandAllocator
    {
    public:
      virtual ~CommandAllocator() = default;

    private:

    };

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
  }
}