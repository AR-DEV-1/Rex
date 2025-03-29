#pragma once

#include "rex_engine/frameinfo/frameinfo.h"

#include "rex_engine/memory/global_allocators/global_allocator.h"
#include "rex_engine/memory/allocators/circular_allocator.h"
#include "rex_engine/memory/allocators/stack_allocator.h"

namespace rex
{
	// #TODO: Remaining cleanup of development/Pokemon -> main merge. ID: REX GLOBALS
	struct Globals
	{
		struct Allocators
		{
			rsl::unique_ptr<TCircularAllocator<GlobalAllocator>> scratch_allocator;		// An allocator used for temp memory. deallocation isn't tracked. Memory may or may not last more than 1 frame
			rsl::unique_ptr<TStackAllocator<GlobalAllocator>> single_frame_allocator;	// An allocator used for memory that's used within a single frame. It gets reset at the beginning of the frame
		};

		Allocators allocators;
		FrameInfo frame_info;
	};

	// Globals are tied together so we have a strict initialization order
	// Having this also avoids the discussion how globals are accessed
	// - No singletons
	// - No local statics
	// - No pure global variables
	// All engine globals are accessed through the below func
	const Globals& globals();
}