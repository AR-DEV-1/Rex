#pragma once

#include "rex_engine/engine/types.h"
#include "rex_engine/assets/tilemap.h"
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

	private:
		rsl::string m_name;
		rex::Tilemap* m_tilemap;
		rsl::pointi32 m_camera_pos;
		f32 m_min_zoom_level;
		f32 m_max_zoom_level;
		f32 m_current_zoom_level;
		f32 m_zoom_step_rate;
	};
}