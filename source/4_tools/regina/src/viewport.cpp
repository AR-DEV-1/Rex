#include "regina/viewport.h"

#include "rex_engine/gfx/rendering/renderer.h"

namespace regina
{
	Viewport::Viewport(rsl::string_view name, rex::Tilemap* tilemap)
		: m_name(name)
		, m_tilemap(tilemap)
		, m_screen_tile_resolution({20, 18})
	{}

	void Viewport::update()
	{
		// based on the camera position, create a tilemap, the same size as the viewport's resolution
		// the camera has a zoom. more tiles are drawn depending on zoom
		rsl::pointi32 top_left = top_left_from_camera_pos(m_camera_pos);

		// The vertex shader needs the following information
		//
		// RenderingMetaData texture_data{};
		// texture_data.texture_tiles_per_row = m_tileset_texture->width() / constants::g_tile_width_px;
		// texture_data.inv_texture_width = 1.0f / m_tileset_texture->width();
		// texture_data.inv_texture_height = 1.0f / m_tileset_texture->height();
		// 
		// texture_data.screen_width_in_tiles = constants::g_screen_width_in_tiles;
		// texture_data.inv_tile_screen_width = 2.0f / constants::g_screen_width_in_tiles;
		// texture_data.inv_tile_screen_height = 2.0f / constants::g_screen_height_in_tiles;
		//
		//

		// The shader needs to know where the tile is on screen and what tile this represents in the texture
		// The strategy for both is the same, converting a tile index into a 2D coordinate
		// For the position, the 2D coordinate is where the vertex is on screen
		// For the UV, the 2D coordinate is the top left of the UV start in the texture
		// So what do we need?
		// list of tiles indices to render
		// number of tiles in a single row in the tileset
		// the inv tile size of a single tile in the tileset
		// number of tiles in a single row on screen
		// the inv tile size of a single tile on the screen

		update_screen_tilemap(top_left);

		rex::gfx::TilemapRenderRequest tilemap_render_request{};

		tilemap_render_request.render_target = m_render_target.get();
		
		tilemap_render_request.tilemap = m_screen_tilemap.get();
		tilemap_render_request.tileset = m_tileset;

		rex::gfx::renderer::instance()->render_tilemap(tilemap_render_request);

		//TilemapInfo info{};

		//// Provide the list of tile indices that we want to render
		//info.tilemap = m_tilemap;

		//// Provide the number of tiles to render in a single row
		//info.tiles_per_row = /**/;

		//// Provide the pixel size of a single tile
		//info.tiles = /**/;
		//info.tile_size = /**/;
		//info.tileset = /**/;
		//

		//rex::gfx::submit_tilemap()
	}

	rsl::pointi32 Viewport::top_left_from_camera_pos(rsl::pointi32 cameraPos)
	{
		// The camera position is in tile coordinates, which will always point to the middle of the screen
		// The goal of this function is to return the tile coordinate in the top left of the screen
		// This gets calculated by converting the viewport width in pixels to viewport width in tiles 
		// using the current zoom level and the number of pixels a single tile takes on screen.
		// Using this information, we subtract half the width and height from the camera pos
		// so it points to the top left of the screen
		cameraPos.x = 120;
		cameraPos.y = 486;

		rsl::pointi32 top_left = cameraPos;
		top_left.x -= m_screen_tile_resolution.x / 2;
		top_left.y -= m_screen_tile_resolution.y / 2;

		top_left.x = rsl::max(top_left.x, 0);
		top_left.y = rsl::max(top_left.y, 0);

		top_left.x = rsl::min(top_left.x, m_tilemap->width_in_tiles() - m_screen_tile_resolution.x);
		top_left.y = rsl::min(top_left.y, m_tilemap->height_in_tiles() - m_screen_tile_resolution.y);

		return top_left;
	}

	void Viewport::update_screen_tilemap(rsl::pointi32 topLeftStart)
	{
		if (m_screen_tilemap->width_in_tiles() != m_screen_tile_resolution.x || m_screen_tilemap->height_in_tiles() != m_screen_tile_resolution.y)
		{
			m_screen_tilemap = rsl::make_unique<rex::Tilemap>(m_screen_tile_resolution.x, m_screen_tile_resolution.y);
		}

		s32 num_tiles_until_end_of_row = m_tilemap->width_in_tiles() - topLeftStart.x;
		s32 num_to_copy = rsl::min(m_screen_tile_resolution.x, num_tiles_until_end_of_row);

		s32 start_idx = topLeftStart.y * m_tilemap->width_in_tiles() + topLeftStart.x;
		const u8* src = m_tilemap->tiles() + start_idx;
		s32 offset = 0;
		for (s32 row = 0; row < m_screen_tile_resolution.y; ++row)
		{
			m_screen_tilemap->set(src, num_to_copy, offset);
			offset += m_screen_tile_resolution.x;
			src += m_tilemap->width_in_tiles();
		}
	}
}