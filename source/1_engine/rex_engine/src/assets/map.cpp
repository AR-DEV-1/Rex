#include "rex_engine/assets/map.h"

#include "rex_engine/engine/asset_db.h"
#include "rex_engine/memory/blob_reader.h"

namespace rex
{
	Map::Map(MapDesc&& desc, LoadFlags loadFlags)
		: m_desc(rsl::move(desc))
	{
		if (!rsl::has_flag(loadFlags, LoadFlags::PartialLoad))
		{
			load_tiles();
		}
	}

	const MapDesc& Map::desc() const
	{
		return m_desc;
	}
	const u8* Map::tiles() const
	{
		return m_tiles.get();
	}

	void Map::load_tiles()
	{
		const s32 num_tiles_per_block = 4;
		m_tiles = rsl::make_unique<u8[]>(m_desc.map_header.width_in_blocks * num_tiles_per_block * m_desc.map_header.height_in_blocks * num_tiles_per_block);

		Blockset* blockset = asset_db::instance()->load<Blockset>(m_desc.blockset);
		memory::Blob blockmap = vfs::instance()->read_file(m_desc.blockmap);

		memory::BlobReader reader(blockmap);
		
		s32 num_blocks_read = 0;
		while (reader.read_offset() < blockmap.size())
		{
			// Reading 1 block
			s32 num_blocks_read = reader.read_offset();
			u8 block_idx = reader.read<u8>();
			const Block& block = blockset->block(block_idx);

			rsl::array<u8, num_tiles_per_block> block_tiles_in_row;
			for (s32 row = 0; row < num_tiles_per_block; ++row)
			{
				// Reading 4 tiles (1 block row)
				block.fill_tiles(row, block_tiles_in_row.data(), block_tiles_in_row.size());
				u8* start = m_tiles.get() + (num_blocks_read * num_tiles_per_block) + (row * m_desc.map_header.width_in_blocks * num_tiles_per_block);
				rsl::memcpy(start, block_tiles_in_row.data(), num_tiles_per_block);
			}
		}
	}

	//s32 Map::width_in_blocks() const
	//{
	//	return m_desc.map_header.width_in_blocks;
	//}
	//s32 Map::height_in_blocks() const
	//{
	//	return m_desc.map_header.height_in_blocks;
	//}
	//s32 Map::border_block_idx() const
	//{
	//	return m_desc.map_header.border_block_idx;
	//}

	//const rsl::unique_array<MapConnection>& Map::connections() const
	//{
	//	return m_desc.connections;
	//}
	//const Blockset* Map::blockset() const
	//{
	//	return m_desc.blocks;
	//}

	//const rsl::unique_array<s8>& Map::blocks() const
	//{
	//	return m_desc.blocks;
	//}
}