#pragma once

#include "rex_engine/engine/types.h"
#include "rex_engine/engine/defines.h"
#include "rex_engine/memory/alloc_unique.h"

#include "rex_engine/memory/memory_types.h"
#include "rex_engine/memory/global_allocators/global_allocator.h"

#include "rex_std/memory.h"
#include "rex_std/stdlib.h"

#include "rex_engine/diagnostics/log.h"

namespace rex
{
	DEFINE_LOG_CATEGORY(LogBlockAllocator);

	//
	// A block allocator allocates a block memory and splits it up into multiple blocks that are all linked together
	// This allows to quickly get a fresh block of memory on allocation without having to look it up
	// 
	//   +-------------------------------------------------------------------------------------------------------------------------+
	//   |    +--------------+				+--------------+				+--------------+				+--------------+				+--------------+     |
	//   |    |              |				|              |				|              |				|              |				|              |		 |
	//   |    |              |				|              |				|              |				|              |				|              |		 |
	//   |    |    Block     |  --> 	|    Block     |  --> 	|    Block     |  --> 	|    Block     |  --> 	|    Block     |		 |
	//   |    |              |				|              |				|              |				|              |				|              |		 |
	//   |    |              |				|              |				|              |				|              |				|              |		 |
	//   |    +--------------+				+--------------+				+--------------+				+--------------+				+--------------+		 |
	//   +-------------------------------------------------------------------------------------------------------------------------+
	//

	namespace internal
	{
		struct BlockHeader
		{
			BlockHeader* next;
		};
	}

	template <typename BackendAllocator>
	class TBlockAllocator
	{
	public:
		using size_type = s64;
		using pointer = void*;

		TBlockAllocator(size_type size, size_type blockSize, BackendAllocator alloc = BackendAllocator())
		{
			m_buffer = alloc_unique<rsl::byte[]>(alloc, size);
			m_block_size = blockSize;
			m_head = reinterpret_cast<internal::BlockHeader*>(m_buffer.get());

			init_blocks();
		}

		REX_NO_DISCARD pointer allocate(size_type size, size_type alignment)
		{
			REX_UNUSED_PARAM(size); // Size cannot be used for buddy allocator
			REX_UNUSED_PARAM(alignment); // Alignment cannot be used for buddy allocator

			REX_ASSERT_X(size <= m_block_size, "Trying to allocate something that's bigger than the block size of a block allocator. alloc size: {} block size: {}", size, m_block_size);

			internal::BlockHeader* current = m_head;
			REX_ASSERT_X(current != nullptr, "Ran out of memory in the free list of the block allocator. Total size: {}", m_buffer.count());

			remove_free_block(current);
			return to_user_pointer(current);
		}
		REX_NO_DISCARD pointer allocate(size_type size)
		{
			u64 alignment = alignof(rsl::max_align);
			return allocate(size, alignment);
		}
		template <typename T>
		REX_NO_DISCARD T* allocate()
		{
			return static_cast<T*>(allocate(sizeof(T), alignof(T)));
		}

		void deallocate(pointer ptr, size_type size)
		{
			REX_UNUSED_PARAM(size);

			rsl::byte* ptr_as_bytes = reinterpret_cast<rsl::byte*>(ptr);
			ptr_as_bytes -= sizeof(internal::BlockHeader);
			internal::BlockHeader* block = reinterpret_cast<internal::BlockHeader*>(ptr_as_bytes);

			block->next = m_head;
			m_head = block;
		}
		template <typename T>
		void deallocate(T* ptr)
		{
			deallocate(ptr, sizeof(T));
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

		bool operator==(const TBlockAllocator& rhs) const
		{
			return m_buffer.get() == rhs.m_buffer.get();
		}
		bool operator!=(const TBlockAllocator& rhs) const
		{
			return !(*this == rhs);
		}

	private:
		void init_blocks()
		{
			REX_ASSERT_X(m_buffer.get() != nullptr, "No backbuffer allocated for block allocator");
			REX_ASSERT_X(m_block_size > 0, "Block size should always be bigger than 0");

			rsl::byte* end = m_buffer.get() + m_buffer.count();
			rsl::byte* current = m_buffer.get();
			s64 rem = m_buffer.count() % (sizeof(internal::BlockHeader) + m_block_size);
			if (rem > 0)
			{
				REX_WARN(LogBlockAllocator, "Block size is not a good denominator for block allocator. increase by {} bytes to be perfectly matched", rem);
				end -= rem;
			}

			internal::BlockHeader* current_block = nullptr;
			while (current != end)
			{
				current_block = reinterpret_cast<internal::BlockHeader*>(current);
				current += sizeof(internal::BlockHeader) + m_block_size;
				internal::BlockHeader* next_block = reinterpret_cast<internal::BlockHeader*>(current);
				
				current_block->next = next_block;
			}
			current_block->next = nullptr;

		}
		pointer to_user_pointer(internal::BlockHeader* block) const
		{
			rsl::byte* ptr = reinterpret_cast<rsl::byte*>(block);
			return ptr += sizeof(internal::BlockHeader);
		}
		void remove_free_block(internal::BlockHeader* toRemove)
		{
			m_head = toRemove->next;
		}

	private:
		rsl::unique_array<rsl::byte, DeleterWithAllocator<rsl::byte, BackendAllocator>> m_buffer;
		size_type m_block_size;
		internal::BlockHeader* m_head;
	};

	using BlockAllocator = TBlockAllocator<GlobalAllocator>;
}