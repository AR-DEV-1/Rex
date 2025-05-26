#include "rex_engine/assets/tileset_asset.h"

namespace rex
{
	const rex::TextureAsset* TilesetAsset::tileset_texture() const
	{
		return m_tileset_texture;
	}
	const rex::Blockset* TilesetAsset::blockset() const
	{
		return m_blockset;
	}

	s32 TilesetAsset::tile_width_px() const
	{
		return m_tile_width_px;
	}
	s32 TilesetAsset::tile_height_px() const
	{
		return m_tile_height_px;
	}
}