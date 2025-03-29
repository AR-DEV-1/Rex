#pragma once

#include "rex_std/bonus/memory.h"

#include "rex_engine/engine/types.h"

namespace rex
{
	struct BootSettings
	{
		s64 single_frame_heap_size = 4_kib;
		s64 scratch_heap_size = 4_kib;
	};
}