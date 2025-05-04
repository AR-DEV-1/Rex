#include "rex_engine/assets/tileset.h"

#include "rex_engine/gfx/resources/texture_2d.h"

namespace rex
{
	Tileset::Tileset(rsl::unique_ptr<gfx::Texture2D> texture)
		: m_texture_resource(rsl::move(texture))
	{}

	//Tileset::Tileset(const u8* tilesetData)
	//	: m_tileset_data(tilesetData)
	//{}

	//const u8* Tileset::tile_pixel_row(u8 tileIdx, s8 pxRow) const
	//{
	//	const u8* tile_data_start = tile_data(tileIdx);

	//	return tile_data_start + (pxRow * s_tileset_width_px);
	//}
	//const u8* Tileset::tile_data(u8 tileIdx) const
	//{
	//	constexpr s8 num_tiles_per_row = static_cast<s8>(s_tileset_width_px / Tile::width_px());;

	//	s32 tx = tileIdx % num_tiles_per_row;
	//	s32 ty = tileIdx / num_tiles_per_row;

	//	s32 tile_offset = Tile::height_px() * ty * (s_tileset_width_px * s_tile_pixel_byte_size) + (tx * Tile::width_px());

	//	return m_tileset_data + tile_offset;
	//}
}