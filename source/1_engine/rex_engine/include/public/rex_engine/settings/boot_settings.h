#pragma once

#include "rex_std/bonus/memory.h"

#include "rex_engine/engine/types.h"

namespace rex
{
	struct BootSettings
	{
		s32 single_frame_heap_size = 4_kib;
		s32 scratch_heap_size = 4_kib;
	};
}