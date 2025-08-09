#pragma once

#include "regina//widgets/widget.h"

#include "regina/scene_manager.h"
#include "regina/viewports_controller.h"
#include "rex_engine/assets/map.h"

#include "rex_std/bonus/math/point.h"
#include "rex_std/vector.h"

struct ImGuiWindow;

namespace regina
{
	struct MinMax
	{
		rsl::string name;
		rsl::pointi32 min;
		rsl::pointi32 max;
		rsl::string name_with_conn;
	};

	struct MapConnectionJson
	{
		rsl::string direction;
		rsl::string map;
		s32 offset;
	};

	struct MapObjectJson
	{
		rsl::string name;
	};

	struct MapObjectEventJson
	{
		rsl::pointi32 pos;
		rsl::string sprite;
		rsl::string movement;
		rsl::string direction;
		rsl::string text;
	};

	struct MapWarpJson
	{
		rsl::pointi32 pos;
		s32 dst_map_id;
		s32 dst_warp_id;
	};

	struct MapBgEventJson
	{
		rsl::pointi32 pos;
		rsl::string text;
	};

	struct MapScriptJson
	{
		rsl::string name;
	};

	struct MapJson
	{
		rsl::string name;
		s32 width;
		s32 height;
		rsl::string blockset;
		rsl::string map_blocks;
		s32 border_block_idx;
		rsl::vector<MapConnectionJson> connections;
		rsl::vector<MapObjectJson> objects;
		rsl::vector<MapObjectEventJson> object_events;
		rsl::vector<MapWarpJson> warps;
		rsl::vector<MapBgEventJson> bg_events;
		rsl::vector<MapScriptJson> scripts;
	};

	class MainEditorWidget : public Widget
	{
	public:
		MainEditorWidget();

		void set_active_scene(Scene* scene);
		void set_active_map(rex::Map* map);

	protected:
		bool on_update() override;

	private:
		void render_menu_bar();
		void render_docking_backpanel();
		void render_widgets();
		void render_imgui_widgets();

		void on_new_active_scene();
		void add_new_viewport();

		bool is_map_in_tilemap(const rex::MapDesc* map);
		void load_maps();
		rsl::vector<MinMax> build_tilemap();
		rsl::pointi32 find_map_pos_in_tilemap(const rex::MapDesc* map);
		void move_camera_to_pos(rsl::pointi32 pos);
		MinMax calc_map_rect(const rex::MapHeader& map, rsl::pointi32 startPos);

		MapJson load_map(rsl::string_view mapPath);

	private:
		bool m_show_imgui_demo;
		bool m_show_imgui_style_editor;

		ImGuiWindow* m_docking_backpanel;
		rsl::vector<rsl::unique_ptr<Widget>> m_widgets;
		ViewportsController m_viewports_controller;

		rex::Map* m_active_map;
		Scene* m_active_scene;

		// This is used to query information about the scene.
		// This info is often passed to widgets
		SceneManager* m_scene_manager;

		std::vector<MapJson> m_map_jsons;
		std::vector<rex::Map*> m_maps;
		rsl::unordered_map<rsl::string, const rex::MapDesc*> m_name_to_map;
	};
}