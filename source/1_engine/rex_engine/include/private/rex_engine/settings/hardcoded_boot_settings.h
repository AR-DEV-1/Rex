#pragma once

#include "rex_std/string_view.h"

namespace rex
{
	rsl::string_view g_hardcoded_boot_ini =
		"[heaps]\n"
		"single_frame_heap_size=4048\n"
		"scratch_heap_size=4048\n";
}