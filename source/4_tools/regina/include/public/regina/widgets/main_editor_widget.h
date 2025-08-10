#pragma once

#include "regina//widgets/widget.h"

#include "regina/scene_manager.h"
#include "regina/viewports_controller.h"
#include "rex_engine/assets/map.h"
#include "rex_engine/assets/tilemap.h"

#include "rex_std/bonus/math/point.h"
#include "rex_std/vector.h"

struct ImGuiWindow;

namespace regina
{
	struct MinMax
	{
		rsl::pointi32 min;
		rsl::pointi32 max;
	};

	struct MapMetaData
	{
		MinMax aabb;
	};

	class MainEditorWidget : public Widget
	{
	public:
		MainEditorWidget();

		void set_active_map(rex::Map* map);

	protected:
		bool on_update() override;

	private:
		void render_menu_bar();
		void render_docking_backpanel();
		void render_widgets();
		void render_imgui_widgets();

		void add_new_viewport();
		void on_new_active_map();

		bool is_map_in_tilemap(const rex::Map* map);
		void load_maps();
		rsl::unordered_map<const rex::Map*, MapMetaData> build_tilemap();
		rsl::pointi32 map_pos(const rex::Map* map);
		void move_camera_to_pos(rsl::pointi32 pos);
		MinMax calc_map_rect(const rex::MapHeader& map, rsl::pointi32 startPos);

	private:
		bool m_show_imgui_demo;
		bool m_show_imgui_style_editor;

		ImGuiWindow* m_docking_backpanel;
		rsl::vector<rsl::unique_ptr<Widget>> m_widgets;
		ViewportsController m_viewports_controller;

		rex::Map* m_active_map;
		rsl::unordered_map<const rex::Map*, MapMetaData> m_map_to_metadata;
		rsl::unique_ptr<rex::Tilemap> m_tilemap;
	};
}