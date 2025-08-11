#include "regina/viewport.h"

namespace regina
{
	Viewport::Viewport(rsl::string_view name, rex::Tilemap* tilemap)
		: m_name(name)
		, m_tilemap(tilemap)
		, m_min_zoom_level(0.1f)
		, m_max_zoom_level(1.0f)
		, m_current_zoom_level(m_max_zoom_level)
		, m_zoom_step_rate(0.125f)
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

		TilemapInfo info{};
		info.tilemap = m_tilemap;
		info.tiles_per_row = /**/;
		info.tiles = /**/;
		info.tile_size = /**/;
		info.tileset = /**/;
		

		rex::gfx::submit_tilemap()
	}

	rsl::pointi32 Viewport::top_left_from_camera_pos(rsl::pointi32 cameraPos)
	{
		// The camera position is in tile coordinates, which will always point to the middle of the screen
		// The goal of this function is to return the tile coordinate in the top left of the screen
		// This gets calculated by converting the viewport width in pixels to viewport width in tiles 
		// using the current zoom level and the number of pixels a single tile takes on screen.
		// Using this information, we subtract half the width and height from the camera pos
		// so it points to the top left of the screen
		s32 widget_width = 0;
		s32 widget_height = 0;
		
		f32 widget_width_in_tiles = widget_width * m_tilemap->tile_width_px() * m_current_zoom_level;
		f32 widget_height_in_tiles = widget_height * m_tilemap->tile_height_px() * m_current_zoom_level;

		rsl::pointi32 top_left = cameraPos;
		top_left.x -= widget_width_in_tiles * 0.5f;
		top_left.y -= widget_height_in_tiles * 0.5f;

		return top_left;
	}

}