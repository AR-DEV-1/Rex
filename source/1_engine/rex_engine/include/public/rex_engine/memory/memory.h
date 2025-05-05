#pragma once

#include "rex_engine/memory/global_allocators/global_allocator.h"
#include "rex_engine/memory/global_allocators/global_debug_allocator.h"
#include "rex_engine/memory/global_allocators/global_scratch_allocator.h"
#include "rex_engine/memory/global_allocators/global_single_frame_allocator.h"

namespace rex
{
	// Allocations that last until they get deallocated
	void* alloc(u64 size);
	void* realloc(void* p, u64 size);
	void dealloc(void* p, u64 size = 0);

	// Allocations that last until they get deallocatoed, but only in debug builds
	void* debug_alloc(u64 size);
	void* debug_realloc(void* p, u64 size);
	void debug_dealloc(void* p, u64 size = 0);

	// Allocations who's lifetime isn't well determined
	// They get allocated from an internal ring buffer
	void* scratch_alloc(u64 size);
	void* scratch_realloc(void* p, u64 size);
	void scratch_dealloc(void* p, u64 size = 0);

	// allocations that only last a single frame
	void* temp_alloc(u64 size);
	void* temp_realloc(void* p, u64 size);
	void temp_dealloc(void* p, u64 size = 0);
}