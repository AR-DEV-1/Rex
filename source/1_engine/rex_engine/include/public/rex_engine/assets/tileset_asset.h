#pragma once

#include "rex_engine/assets/asset.h"
#include "rex_engine/assets/texture_asset.h"

#include "rex_engine/assets/blockset.h"

namespace rex
{
	// To render a tileset, you need a few things
	// 1) the tileset texture, obviously
	// 2) the blockset
	//		  an abstraction on top. This allows for more modular. A block is a 4x4 matrix holding tile indices into the tileset texture
	// 3) tile dimensions, holding the width and height of a single tile, in pixels
	class TilesetAsset : public Asset
	{
	public:
		const rex::TextureAsset* tileset_texture() const;
		const rex::Blockset* blockset() const;

		s32 tile_width_px() const;
		s32 tile_height_px() const;

	private:
		const rex::TextureAsset* m_tileset_texture;
		const rex::Blockset* m_blockset;

		// Store how big the tiles are in pixels
		s32 m_tile_width_px;
		s32 m_tile_height_px;
	};

}