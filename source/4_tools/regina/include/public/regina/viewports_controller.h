#pragma once

#include "regina/widgets/viewport_widget_old.h"
#include "rex_engine/gfx/rendering/render_passes/tile_pass.h"

#include "rex_engine/assets/map.h"

namespace regina
{
	class Scene;
	class ViewportWidget;

	//struct MinMax
	//{
	//	rsl::pointi32 min;
	//	rsl::pointi32 max;
	//};

	// The viewports controllers manages the update of 1 or more viewports.
	// It makes sure the camera matrices are set correctly and makes sure the gpu has all the data it needs
	class ViewportsController
	{
	public:
		ViewportsController(Scene* activeScene);

		void add_new_viewport();

		void update();

	private:
		void process_input();
		void update_renderpass_data();
		void draw_imgui();

		TileMap build_backend_tilemap(const rsl::vector<rex::MapDesc>& maps);
		//void calc_map_rects(const rex::MapDesc& map, rsl::pointi32 startPos, rsl::vector<const rex::MapDesc*>& visitedMaps, rsl::vector<MinMax>& mapRects);
		//MinMax calc_map_rect(const rex::MapHeader& map, rsl::pointi32 startPos);

	private:
		ViewportWidget* m_viewport_widget;
		//rex::gfx::TilePass* m_render_pass;
		Scene* m_active_scene;
	};
}