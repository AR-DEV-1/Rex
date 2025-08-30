#pragma once

#include "rex_engine/memory/blob_view.h"

#include "rex_engine/engine/types.h"

namespace rex
{
	struct ImageLoadResult
	{
		s32 width;
		s32 height;
		s32 num_channels;
		rsl::unique_array<u8> data;
	};

	ImageLoadResult load_image(memory::BlobView content);
}