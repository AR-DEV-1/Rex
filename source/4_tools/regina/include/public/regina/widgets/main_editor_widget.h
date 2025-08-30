#pragma once

#include "regina/widgets/widget.h"
#include "regina/widgets/viewport.h"

#include "regina/scene_manager.h"
#include "regina/viewports_controller.h"
#include "regina/world_composer.h"
#include "rex_engine/assets/map.h"
#include "rex_engine/assets/tilemap.h"
#include "rex_engine/assets/tileset_asset.h"

#include "rex_std/bonus/math/point.h"
#include "rex_std/vector.h"

struct ImGuiWindow;

namespace regina
{
	class MainEditorWidget : public Widget
	{
	public:
		MainEditorWidget();

		void set_active_map(rex::Map* map);

	protected:
		bool on_update() override;
		void on_draw() override;

	private:
		void update_widgets();

		void draw_menu_bar();
		void draw_docking_backpanel();
		void draw_widgets();
		void draw_imgui_widgets();

		void on_new_active_map();

		void move_camera_to_pos(rsl::pointi32 pos);

	private:
		bool m_show_imgui_demo;
		bool m_show_imgui_style_editor;

		ImGuiWindow* m_docking_backpanel;
		rsl::vector<rsl::unique_ptr<Widget>> m_widgets;
		ViewportsController m_viewports_controller;

		rex::Map* m_active_map;
		WorldComposer m_world_composer;
		rex::TilesetAsset* m_tileset;

		Viewport* m_viewport;
	};
}