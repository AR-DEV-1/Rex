#pragma once

#include "rex_engine/engine/types.h"
#include "rex_engine/assets/tilemap.h"
#include "rex_engine/assets/tileset.h"
#include "rex_std/string.h"
#include "rex_std/bonus/math.h"

namespace regina
{
	class Viewport
	{
	public:
		Viewport(rsl::string_view name, rex::Tilemap* tilemap);

		void update();

		rsl::pointi32 top_left_from_camera_pos(rsl::pointi32 cameraPos);
		void update_screen_tilemap(rsl::pointi32 topLeftStart);

	private:
		// Give a name to the viewport, making it easier to identify
		rsl::string m_name;

		// The tileset to use for the viewport
		// We currently only support 1 tileset per viewport
		rex::Tileset* m_tileset;

		// the tilemap of the entire world, only a subset of this is rendered at a time
		rex::Tilemap* m_tilemap;

		// The tilemap off the render target itself, which is a subset of the tilemap of the entire world
		rsl::unique_ptr<rex::Tilemap> m_screen_tilemap;

		// The current camera position in the world, offsetted from the top left of the world
		rsl::pointi32 m_camera_pos;
		rsl::pointi32 m_screen_tile_resolution;

		rsl::unique_ptr<rex::gfx::RenderTarget> m_render_target;
	};
}