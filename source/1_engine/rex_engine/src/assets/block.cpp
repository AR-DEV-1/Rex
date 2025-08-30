#include "rex_engine/assets/block.h"
#include "rex_engine/assets/tileset.h"

namespace rex
{
	Block::Block()
	{
		m_tile_indices.fill(s_invalid_tile_index);
	}
	Block::Block(const indices_array& tileIndices)
		: m_tile_indices(tileIndices)
	{

	}

	void Block::set_all(s8 idx)
	{
		m_tile_indices.fill(idx);
	}

	u8 Block::index_at(s8 idx) const
	{
		return m_tile_indices[idx];
	}
	u8 Block::index_at(rsl::pointi8 coord) const
	{
		s8 idx = coord.y * s_num_tiles_per_row + coord.x;
		return m_tile_indices[idx];
	}

	void Block::fill_tiles(s32 rowIdx, u8* dstBuff, s32 dstBuffSize) const
	{
		const u8* start = m_tile_indices.data() + (rowIdx * s_num_tiles_per_row);
		rsl::memcpy(dstBuff, start, rsl::min<s32>(dstBuffSize, s_num_tiles_per_row));
	}

	rsl::random_access_iterator<u8> Block::begin()
	{
		return m_tile_indices.begin();
	}
	rsl::const_random_access_iterator<u8> Block::begin() const
	{
		return m_tile_indices.begin();
	}
	rsl::const_random_access_iterator<u8> Block::cbegin() const
	{
		return m_tile_indices.cbegin();
	}

	rsl::random_access_iterator<u8> Block::end()
	{
		return m_tile_indices.end();
	}
	rsl::const_random_access_iterator<u8> Block::end() const
	{
		return m_tile_indices.end();
	}
	rsl::const_random_access_iterator<u8> Block::cend() const
	{
		return m_tile_indices.cend();
	}
}