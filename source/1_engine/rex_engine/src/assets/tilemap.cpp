#include "rex_engine/assets/tilemap.h"

#include "rex_engine/diagnostics/assert.h"

namespace rex
{
	Tilemap::Tilemap(s32 width, s32 height)
		: m_width(width)
		, m_height(height)
	{
		m_tiles = rsl::make_unique<u8[]>(m_width * m_height);
	}

	void Tilemap::set(const u8* data, s32 len, s32 offset)
	{
		REX_ASSERT_X(offset + len < m_tiles.count(), "would write out of bounds of tiles array");

		rsl::memcpy(m_tiles.get() + offset, data, len);
	}
}