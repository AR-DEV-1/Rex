#include "regina/widgets/main_editor_widget.h"

#include "regina/widgets/content_browser_widget.h"
#include "regina/widgets/viewport_widget.h"

#include "rex_engine/event_system/event_system.h"
#include "rex_engine/event_system/events/app/quit_app.h"

#include "rex_engine/engine/asset_db.h"
#include "rex_engine/gfx/imgui/imgui_utils.h"
#include "rex_engine/gfx/imgui/imgui_scoped_widget.h"
#include "rex_engine/filesystem/path.h"
#include "rex_engine/filesystem/vfs.h"
#include "rex_engine/filesystem/file.h"
#include "rex_engine/filesystem/directory.h"
#include "rex_engine/text_processing/json.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

DEFINE_LOG_CATEGORY(LogMainEditor);

namespace regina
{
	// +--------------+-------------------------------------------------+-------------+
	// |							| 																								| 					  |
	// |							|  																								| 					  |
	// |							|  																								| 					  |
	// |							|  																								| 					  |
	// |							|  																								| 					  |
	// |							|  																								| 					  |
	// |							|  																								| 					  |
	// +--------+-----+-------------------------------------------------+-------------+
	// |				| 																																	  |
	// |				| 																																	  |
	// |				| 																																	  |
	// |				| 																																	  |
	// +--------+---------------------------------------------------------------------+


	MainEditorWidget::MainEditorWidget()
		: m_show_imgui_demo(false)
		, m_show_imgui_style_editor(false)
		, m_scene_manager(nullptr)
		, m_viewports_controller(nullptr)
		, m_active_map(nullptr)
	{
		//ImGuiIO& io = ImGui::GetIO();
		//if (!rex::file::exists(rsl::string_view(io.IniFilename)))
		//{
			rex::scratch_string main_layout_settings = rex::path::join(rex::vfs::instance()->root(), "Regina", "main_editor_layout.ini");
			ImGui::LoadIniSettingsFromDisk(main_layout_settings.data());
		//}

		m_widgets.emplace_back(rsl::make_unique<ContentBrowserWidget>());

		// Always start with 1 viewport active by default
		add_new_viewport();
	}

	void MainEditorWidget::set_active_scene(Scene* scene)
	{
		if (m_active_scene == scene)
		{
			return;
		}

		m_active_scene = scene;
		on_new_active_scene();
	}

	void MainEditorWidget::set_active_map(rex::Map* map)
	{
		if (m_active_map == map)
		{
			return;
		}

		m_active_map = map;
		on_new_active_scene();
	}

	bool MainEditorWidget::on_update()
	{
		render_menu_bar();
		render_docking_backpanel();
		m_viewports_controller.update();
	
		render_widgets();

		render_imgui_widgets();

		return false;
	}

	void MainEditorWidget::render_menu_bar()
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Quit"))
				{
					rex::event_system::instance()->enqueue_event(rex::QuitApp("Quit Pressed In Menu"));
				}
				if (ImGui::MenuItem("Show ImGui Demo", false, &m_show_imgui_demo))
				{
					m_show_imgui_demo = !m_show_imgui_demo;
				}
				if (ImGui::MenuItem("Show ImGui Style Editor", false, &m_show_imgui_style_editor))
				{
					m_show_imgui_style_editor = !m_show_imgui_style_editor;
				}

				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
	}
	void MainEditorWidget::render_docking_backpanel()
	{
		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;

		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
		// and handle the pass-thru hole, so we ask Begin() to not render a background.
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		{
			window_flags |= ImGuiWindowFlags_NoBackground;
		}

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Docking Backpanel", nullptr, window_flags);
		m_docking_backpanel = ImGui::GetCurrentWindow();
		ImGui::PopStyleVar();

		ImGui::PopStyleVar(2);

		// Submit the DockSpace
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("Regina Dockspace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		ImGui::End();
	}
	void MainEditorWidget::render_widgets()
	{
		for (auto& widget : m_widgets)
		{
			widget->update();
		}

		if (auto widget = rex::imgui::ScopedWidget("Scene Hierachy"))
		{
			ImGui::Text("This is the scene hiearchy");
		}

		if (auto widget = rex::imgui::ScopedWidget("Properties"))
		{
			ImGui::Text("This is the properties panel");
		}
	}

	void MainEditorWidget::on_new_active_scene()
	{
		if (!is_map_in_tilemap(&m_active_map->desc()))
		{
			// 1. Load the active map and all connections recursively for all maps within the current area (eg. load all maps in kanto)
			load_maps();

			// 2. Calculate the AABB for each map, converted to absolute coordinates and cache these
			build_tilemap();
			// The min max results are stored relative from the active map
			// We need to convert them to absolute positions
			// We do this by getting the lowest possible point in the relative position
			// and converting that point to be our origin
			rsl::pointi32 min_pos{};
			for (const auto& [name, minmax] : m_name_to_aabb)
			{
				min_pos.x = rsl::min(min_pos.x, minmax.min.x);
				min_pos.y = rsl::min(min_pos.y, minmax.min.y);
			}

			// Now go over all the minmax results and convert their coordinates
			for (auto& [name, minmax] : m_name_to_aabb)
			{
				minmax.min.x -= min_pos.x;
				minmax.min.y -= min_pos.y;
				minmax.max.x -= min_pos.x;
				minmax.max.y -= min_pos.y;
			}
		}

		// 3. Move the camera to the active map
		rsl::pointi32 pos_in_tilemap = m_name_to_aabb.at(m_active_map->desc().map_header.name).min;
		move_camera_to_pos(pos_in_tilemap);
	}
	void MainEditorWidget::add_new_viewport()
	{
		//ViewportWidget* new_viewport = m_widgets.emplace_back(rsl::make_unique<ViewportWidget>(m_scene_manager->current_scene())).get();
		//m_viewports_controller.add_new_viewport(new_viewport);
	}
	void MainEditorWidget::render_imgui_widgets()
	{
		if (m_show_imgui_demo)
		{
			ImGui::ShowDemoWindow();
		}
		if (m_show_imgui_style_editor)
		{
			rex::imgui::ScopedWidget widget("ImGui Style Editor");
			ImGui::ShowStyleEditor();
		}
	}

	bool MainEditorWidget::is_map_in_tilemap(const rex::MapDesc* map)
	{
		if (!map)
		{
			return false;
		}

		return m_name_to_map.contains(map->map_header.name);
	}

	void MainEditorWidget::load_maps()
	{
		// load the active map and all its connections, recursively until there are none left
		rsl::vector<rsl::string> open_nodes;
		rsl::vector<rsl::string> closed_nodes;

		rsl::string_view asset_path = rex::asset_db::instance()->asset_path(m_active_map);
		open_nodes.push_back(rsl::string(asset_path));

		while (!open_nodes.empty())
		{
			rsl::string current_node = rsl::move(open_nodes.back());
			open_nodes.pop_back();

			// Do not reload a map if it's already progressed
			if (rsl::find(closed_nodes.cbegin(), closed_nodes.cend(), current_node) != closed_nodes.cend())
			{
				continue;
			}

			// Load a map and add all its connections to the open node
			MapJson map = load_map(current_node);
			for (const MapConnectionJson& conn : map.connections)
			{
				open_nodes.push_back(rsl::string(rex::path::join(rex::engine::instance()->data_root(), conn.map)));
			}

			m_map_jsons.push_back(rsl::move(map));
			m_maps.push_back(rex::asset_db::instance()->load_from_json<rex::Map>(current_node));

			for (const rex::MapConnection& conn : m_maps.back()->desc().connections)
			{
				rex::asset_db::instance()->hydra_asset(conn.map);
			}

			closed_nodes.push_back(current_node);
		}
	}

	void MainEditorWidget::build_tilemap()
	{
		struct MapWithPos
		{
			const rex::MapDesc* desc;
			rsl::pointi32 pos;
		};
		rsl::pointi32 start_pos{};
		std::vector<MapWithPos> open_nodes;

		open_nodes.push_back({ &m_active_map->desc(), start_pos});
		m_name_to_aabb.clear();

		std::vector<MapWithPos> closed_nodes;
		rsl::unordered_map<rsl::string_view, const rex::MapDesc*> name_to_map;
		name_to_map.reserve(m_map_jsons.size());

		while (!open_nodes.empty())
		{
			MapWithPos current_node = open_nodes.back();
			open_nodes.pop_back();
			auto closed_node_it = std::find_if(closed_nodes.cbegin(), closed_nodes.cend(), [&](const MapWithPos& mapWithPos)
				{
					return mapWithPos.desc->map_header.name == current_node.desc->map_header.name;
				});
			if (closed_node_it != closed_nodes.cend())
			{
				continue;
			}

			start_pos = current_node.pos;

			// 1. calculate bounding box of current map
			MinMax map_rect = calc_map_rect(current_node.desc->map_header, current_node.pos);
			m_name_to_aabb.emplace(current_node.desc->map_header.name, map_rect);

			// 2. add the map itself to the closed nodes
			closed_nodes.push_back(current_node);

			// 3. add the map's connections to the open nodes
			for (const rex::MapConnection& conn : current_node.desc->connections)
			{
				auto closed_node_it = std::find_if(closed_nodes.cbegin(), closed_nodes.cend(), [&](const MapWithPos& mapWithPos) 
					{
						return mapWithPos.desc->map_header.name == conn.map->desc().map_header.name;
					});
				if (closed_node_it != closed_nodes.cend())
				{
					continue;
				}

				auto map_desc_it = std::find_if(m_maps.cbegin(), m_maps.cend(), [&](rex::Map* map) 
					{
						return map->desc().map_header.name == conn.map->desc().map_header.name;
					});

				const rex::MapDesc* conn_map = &(*map_desc_it)->desc();
				if (rsl::find_if(closed_nodes.cbegin(), closed_nodes.cend(), [&](const MapWithPos& mapWithPos) { return mapWithPos.desc == conn_map; }) == closed_nodes.cend())
				{
					const s32 tiles_per_block = 4;
					s32 half_width_in_tiles = (current_node.desc->map_header.width_in_blocks * tiles_per_block / 2);
					s32 half_height_in_tiles = (current_node.desc->map_header.height_in_blocks * tiles_per_block / 2);
					s32 half_conn_width_in_tiles = (conn.map->desc().map_header.width_in_blocks * tiles_per_block / 2);
					s32 half_conn_height_in_tiles = (conn.map->desc().map_header.height_in_blocks * tiles_per_block / 2);
					rsl::pointi32 conn_pos;
					if (conn.direction == rex::Direction::North)
					{
						conn_pos = { start_pos.x + conn.offset, start_pos.y + half_height_in_tiles + half_conn_height_in_tiles };
					}
					else if (conn.direction == rex::Direction::South)
					{
						conn_pos = { start_pos.x + conn.offset, start_pos.y - half_height_in_tiles - half_conn_height_in_tiles };
					}

					// Positive offset -> move conn map down
					// Negative offset -> move conn map up

					else if (conn.direction == rex::Direction::East)
					{
						conn_pos = { start_pos.x + half_width_in_tiles + half_conn_width_in_tiles, start_pos.y - conn.offset}; // offsets are stored in tiles
					}
					else if (conn.direction == rex::Direction::West)
					{
						conn_pos = { start_pos.x - half_width_in_tiles - half_conn_width_in_tiles, start_pos.y - conn.offset}; // offsets are stored in tiles
					}
					open_nodes.push_back({ conn_map, conn_pos });
				}
			}
		}
	}

	MinMax MainEditorWidget::calc_map_rect(const rex::MapHeader& map, rsl::pointi32 startPos)
	{
		const s32 tiles_per_block = 4;

		s32 half_width_in_tiles = (map.width_in_blocks * tiles_per_block / 2);
		s32 half_height_in_tiles = (map.height_in_blocks * tiles_per_block / 2);

		MinMax res{};
		res.min.x = -half_width_in_tiles + startPos.x;
		res.min.y = -half_height_in_tiles + startPos.y;
		res.max.x = half_width_in_tiles + startPos.x;
		res.max.y = half_height_in_tiles + startPos.y;

		return res;
	}

	MapJson MainEditorWidget::load_map(rsl::string_view mapPath)
	{
		rex::json::json map_json = rex::json::read_from_file(mapPath);
		MapJson map{};

		map.name = map_json["name"];
		map.width = map_json["width"];
		map.height = map_json["height"];
		map.blockset = map_json["blockset"];
		map.map_blocks = map_json["map_blocks"];
		map.border_block_idx = map_json["border_block_idx"];
		map.connections.reserve(map_json["connections"].size());
		for (const rex::json::json& conn : map_json["connections"])
		{
			MapConnectionJson& connection = map.connections.emplace_back();
			connection.direction = conn["direction"];
			connection.map = conn["map"];
			connection.offset = conn["offset"];
		}
		map.objects.reserve(map_json["objects"].size());
		for (const rex::json::json& obj_json : map_json["objects"])
		{
			MapObjectJson& object = map.objects.emplace_back();
			object.name = obj_json["name"];
		}
		map.object_events.reserve(map_json["object_events"].size());
		for (const rex::json::json& obj_ev_json : map_json["object_events"])
		{
			MapObjectEventJson& object_ev = map.object_events.emplace_back();
			object_ev.pos.x = obj_ev_json["x"];
			object_ev.pos.y = obj_ev_json["y"];
			object_ev.sprite = obj_ev_json["sprite"];
			object_ev.movement = obj_ev_json["movement"];
			object_ev.direction = obj_ev_json["direction"];
			object_ev.text = obj_ev_json["text"];
		}
		map.warps.reserve(map_json["warps"].size());
		for (const rex::json::json& warp_json : map_json["warps"])
		{
			MapWarpJson& warp = map.warps.emplace_back();
			warp.pos.x = warp_json["x"];
			warp.pos.y = warp_json["y"];
			warp.dst_map_id = warp_json["dst_map_id"];
			warp.dst_warp_id = warp_json["dst_warp_id"];
		}
		map.bg_events.reserve(map_json["bg_events"].size());
		for (const rex::json::json& bg_ev_json : map_json["bg_events"])
		{
			MapBgEventJson& bg_event = map.bg_events.emplace_back();
			bg_event.pos.x = bg_ev_json["x"];
			bg_event.pos.y = bg_ev_json["y"];
			bg_event.text = bg_ev_json["text"];
		}
		map.scripts.reserve(map_json["scripts"].size());
		for (const rex::json::json& script_json : map_json["scripts"])
		{
			MapScriptJson& script = map.scripts.emplace_back();
			script.name = script_json;
		}

		return map;
	}

	void MainEditorWidget::move_camera_to_pos(rsl::pointi32 pos)
	{

	}
}