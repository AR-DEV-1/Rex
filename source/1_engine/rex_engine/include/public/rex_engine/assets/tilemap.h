#pragma once

#include "rex_engine/engine/types.h"
#include "rex_std/memory.h"

namespace rex
{
	class Tilemap
	{
	public:
		Tilemap(s32 width, s32 height);

		void set(const u8* data, s32 len, s32 offset);

	private:
		s32 m_width;
		s32 m_height;
		rsl::unique_array<u8> m_tiles;
	};
}