#pragma once

#include "rex_engine/engine/types.h"
#include "rex_engine/engine/defines.h"
#include "rex_engine/memory/alloc_unique.h"
#include "rex_engine/memory/global_allocators/global_allocator.h"

#include "rex_std/memory.h"
#include "rex_std/bonus/memory/zero_memory.h"

namespace rex
{
	// Ring buffer implementation
	template <typename BackendAllocator>
	class TCircularAllocator
	{
	public:
		using size_type = s64;
		using pointer = void*;

		explicit TCircularAllocator(size_type size, BackendAllocator alloc = BackendAllocator())
			: m_buffer(alloc_unique<rsl::byte[]>(alloc, size))
			, m_current(m_buffer.get())
			, m_end(m_buffer.get() + m_buffer.count())
		{
		}

		REX_NO_DISCARD pointer allocate(size_type size, s32 alignment)
		{
			REX_ASSERT_X(size <= m_buffer.count(), "Allocating more data the circular buffer holds.");

			// As we're not given a type, we need to make sure our memory is aligned within the buffer
			m_current = align(m_current, alignment);

			s32 space_available_until_end = narrow_cast<s32>(m_end - m_current);
			if (space_available_until_end < size)
			{
				// If we can't fit the entire allocation
				// We need to reset the allocation pointer to the front
				// this will introduce some wasted memory usage
				// but is better for performance as data will be aligned
				// and it's more cache friendly as well
				m_current = m_buffer.get();
				m_current = align(m_current, alignment);
			}

			pointer result = m_current;
			m_current += size;

			return result;
		}

		REX_NO_DISCARD pointer allocate(size_type size)
		{
			s32 alignment = narrow_cast<s32>(alignof(rsl::max_align));
			return allocate(size, alignment);
		}

		template <typename T>
		REX_NO_DISCARD T* allocate()
		{
			return static_cast<T*>(allocate(sizeof(T), alignof(T)));
		}

		void deallocate(pointer ptr, size_type size = 0)
		{
			// Nothing to implement

			REX_UNUSED_PARAM(ptr);
			REX_UNUSED_PARAM(size);
		}

		template <typename U, typename... Args>
		void  construct(U* p, Args&&... args)
		{
			new (p) (rsl::forward<Args>(args)...);
		}
		template <typename T>
		void destroy(T* ptr)
		{
			ptr->~T();
		}

		bool has_allocated_ptr(void* ptr) const
		{
			return m_buffer.get() <= ptr && ptr < m_end;
		}

		bool operator==(const TCircularAllocator& rhs) const
		{
			return m_buffer.get() == rhs.m_buffer.get();
		}
		bool operator!=(const TCircularAllocator& rhs) const
		{
			return !(*this == rhs);
		}

	private:
		rsl::unique_array<rsl::byte, DeleterWithAllocator<rsl::byte, BackendAllocator>> m_buffer;
		rsl::byte* m_current;
		const rsl::byte* m_end;
	};

	using CircularAllocator = TCircularAllocator<GlobalAllocator>;
}