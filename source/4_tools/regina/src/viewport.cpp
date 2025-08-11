#include "regina/viewport.h"

namespace regina
{
	Viewport::Viewport(rsl::string_view name, rex::Tilemap* tilemap)
		: m_name(name)
		, m_tilemap(tilemap)
		, m_min_pixels_per_tile(1.0f) // Can't zoom further out than 1 pixel per tile
		, m_max_pixels_per_tile(8.0f) // Can't zoom in beyond 8 pixels per tile as this is the zoom of the gameboy
		, m_zoom_step_rate(0.125f)
		, m_current_num_pixels_per_tile(m_max_pixels_per_tile)
	{}

	void Viewport::update()
	{
		// based on the camera position, create a tilemap, the same size as the viewport's resolution
		// the camera has a zoom. more tiles are drawn depending on zoom

		rsl::pointi32 top_left = top_left_from_camera_pos(m_camera_pos);
	}

	rsl::pointi32 Viewport::top_left_from_camera_pos(rsl::pointi32 cameraPos)
	{
		s32 widget_width = 0;
		s32 widget_height = 0;

		f32 num_pixels_per_tile = static_cast<s32>(m_current_num_pixels_per_tile);
		f32 widget_width_in_tiles = widget_width * num_pixels_per_tile;
		f32 widget_height_in_tiles = widget_height * num_pixels_per_tile;

		rsl::pointi32 top_left = cameraPos;
		top_left.x -= widget_width_in_tiles * 0.5f;
		top_left.y -= widget_height_in_tiles * 0.5f;

		return top_left;
	}

}