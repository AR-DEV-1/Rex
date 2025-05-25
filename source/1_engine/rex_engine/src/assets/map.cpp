#include "rex_engine/assets/map.h"

namespace rex
{
	Map::Map(MapDesc&& desc)
		: m_desc(rsl::move(desc))
	{}

	s32 Map::width_in_blocks() const
	{
		return m_desc.map_header.width_in_blocks;
	}
	s32 Map::height_in_blocks() const
	{
		return m_desc.map_header.height_in_blocks;
	}
	s32 Map::border_block_idx() const
	{
		return m_desc.map_header.border_block_idx;
	}

	const rsl::unique_array<MapConnection>& Map::connections() const
	{
		return m_desc.connections;
	}
}