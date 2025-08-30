#include "rex_engine/assets/tileset_asset.h"

namespace rex
{
	TilesetAsset::TilesetAsset(rsl::pointi8 tileSize, const Tileset* texture)
		: m_tile_size(tileSize)
		, m_tileset_texture(texture)
	{
		
	}

	const Tileset* TilesetAsset::tileset_texture() const
	{
		return m_tileset_texture;
	}
	//const rex::Blockset* TilesetAsset::blockset() const
	//{
	//	return m_blockset;
	//}

	rsl::pointi8 TilesetAsset::tile_size() const
	{
		return m_tile_size;
	}
}