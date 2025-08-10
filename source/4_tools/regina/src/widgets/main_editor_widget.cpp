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

	void MainEditorWidget::set_active_map(rex::Map* map)
	{
		if (m_active_map == map)
		{
			return;
		}

		m_active_map = map;
		on_new_active_map();
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

	void MainEditorWidget::on_new_active_map()
	{
		if (!is_map_in_tilemap(m_active_map))
		{
			// 1. Load the active map and all connections recursively for all maps within the current area (eg. load all maps in kanto)
			load_maps();

			// 2. Calculate the AABB for each map, converted to absolute coordinates and cache these
			m_map_to_metadata = build_tilemap();
		}

		// 3. Move the camera to the active map
		rsl::pointi32 pos_in_tilemap = map_pos(m_active_map);
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

	bool MainEditorWidget::is_map_in_tilemap(const rex::Map* map)
	{
		if (!map)
		{
			return false;
		}

		return m_map_to_metadata.contains(map);
	}

	void MainEditorWidget::load_maps()
	{
		// load the active map and all its connections, recursively until there are none left
		// It's not required to do this here, we can load it in the tilemap building as well

		rsl::vector<rex::Map*> open_nodes = { m_active_map };
		rsl::vector<const rex::Map*> closed_nodes;

		while (!open_nodes.empty())
		{
			rex::Map* current_node = open_nodes.back();
			open_nodes.pop_back();
			if (rsl::find(closed_nodes.cbegin(), closed_nodes.cend(), current_node) != closed_nodes.cend())
			{
				continue;
			}

			closed_nodes.push_back(current_node);
			rex::asset_db::instance()->hydrate_asset(current_node);

			for (const rex::MapConnection& conn : current_node->desc().connections)
			{
				open_nodes.push_back(conn.map);
			}
		}
	}

	rsl::unordered_map<const rex::Map*, MapMetaData> MainEditorWidget::build_tilemap()
	{
		// Loop over all the maps we have, starting from the first and save their relative position
		// Afterwards, creating a AABB over that encapsulates all the AABBs of every map and change the origin to point to the top left of the encapsulating AABB
		//
		//                                              --- EXAMPLE ---
		// In the example below, we start from map A and recursively loop over all connections until we've explored all maps
		// this means that all map AABB would be relative from map A
		// we need to calculate the AABB that encapsulates all maps and then recalculate the AABB of each map relative to the origin of the encapsulating AABB
		//
		// +-----------------------------------------------------------------------------------------------------------------------+
		// |																																																											 |
		// |																																					+--------------------------+								 |
		// |		+--------------------------+																					|													 |								 |
		// |		|                          |																					|													 |								 |
		// |		|                          |																					|													 |								 |
		// |		|             A            |  ----------------------------------------|					  	C						 |								 |
		// |		|                          |																					|													 |								 |
		// |		|                          |																					|													 |								 |
		// |		|                          |																					|													 |								 |
		// |		+--------------------------+																					+--------------------------+								 |
		// |									|                                                                     |															 |
		// |									|																																			|															 |
		// |									|																																			|															 |
		// |									|																																			|															 |
		// |									|																																			|															 |
		// |									|																																			|															 |
		// |									|																																			|															 |
		// |									|																																			|															 |
		// |									|																																			|															 |
		// |									|																																			|															 |
		// |								  |																																			|									  					 |
		// |		+--------------------------+																					+--------------------------+								 |
		// |		|                          | 																					|                          |								 |
		// |		|                          | 																					|                          |								 |
		// |		|                          | 																					|                          |								 |
		// |		|             B            | ---------------------------------------- |             D            |	  						 |
		// |		|                          | 																					|                          |								 |
		// |		|                          | 																					|                          |								 |
		// |		|                          | 																					|                          |								 |
		// |		+--------------------------+																					+--------------------------+								 |
		// |																																																											 |
		// |																																																											 |
		// +-----------------------------------------------------------------------------------------------------------------------+
		// 
		// 
		// 
		// 
		//

		struct MapWithPos
		{
			const rex::Map* map;
			rsl::pointi32 pos;
		};
		std::vector<MapWithPos> open_nodes;
		std::vector<MapWithPos> closed_nodes;

		open_nodes.push_back({ m_active_map, rsl::pointi32{} });
		rsl::unordered_map<const rex::Map*, MapMetaData> map_to_metadata;

		while (!open_nodes.empty())
		{
			MapWithPos current_node = open_nodes.back();
			open_nodes.pop_back();

			// skip any maps that are already processed
			auto closed_node_it = std::find_if(closed_nodes.cbegin(), closed_nodes.cend(), [&](const MapWithPos& mapWithPos)
				{
					return mapWithPos.map == current_node.map;
				});
			if (closed_node_it != closed_nodes.cend())
			{
				continue;
			}

			// 1. calculate bounding box of current map
			MinMax map_rect = calc_map_rect(current_node.map->desc().map_header, current_node.pos);
			MapMetaData map_meta_data{};
			map_meta_data.aabb = map_rect;
			map_to_metadata.emplace(current_node.map, map_meta_data);

			// 2. add the map itself to the closed nodes
			closed_nodes.push_back(current_node);

			// 3. add the map's connections to the open nodes
			for (const rex::MapConnection& conn : current_node.map->desc().connections)
			{
				// skip any maps that are already processed
				auto closed_node_it = std::find_if(closed_nodes.cbegin(), closed_nodes.cend(), [&](const MapWithPos& mapWithPos)
					{
						return mapWithPos.map == conn.map;
					});
				if (closed_node_it != closed_nodes.cend())
				{
					continue;
				}

				const s32 tiles_per_block = 4;
				s32 half_width_in_tiles = (current_node.map->desc().map_header.width_in_blocks * tiles_per_block / 2);
				s32 half_height_in_tiles = (current_node.map->desc().map_header.height_in_blocks * tiles_per_block / 2);
				s32 half_conn_width_in_tiles = (conn.map->desc().map_header.width_in_blocks * tiles_per_block / 2);
				s32 half_conn_height_in_tiles = (conn.map->desc().map_header.height_in_blocks * tiles_per_block / 2);
				rsl::pointi32 conn_pos;

				// calculate the middle position of the connecting map
				if (conn.direction == rex::Direction::North)
				{
					conn_pos = { current_node.pos.x + conn.offset, current_node.pos.y + half_height_in_tiles + half_conn_height_in_tiles };
				}
				else if (conn.direction == rex::Direction::South)
				{
					conn_pos = { current_node.pos.x + conn.offset, current_node.pos.y - half_height_in_tiles - half_conn_height_in_tiles };
				}
				else if (conn.direction == rex::Direction::East)
				{
					conn_pos = { current_node.pos.x + half_width_in_tiles + half_conn_width_in_tiles, current_node.pos.y - conn.offset }; // offsets are stored in tiles
				}
				else if (conn.direction == rex::Direction::West)
				{
					conn_pos = { current_node.pos.x - half_width_in_tiles - half_conn_width_in_tiles, current_node.pos.y - conn.offset }; // offsets are stored in tiles
				}

				open_nodes.push_back({ conn.map, conn_pos });
			}
		}

		// The min max results are stored relative from the active map
		// We need to convert them to absolute positions
		// We do this by getting the lowest possible point in the relative position
		// and converting that point to be our origin
		MinMax big_aabb;
		for (const auto& [name, metadata] : map_to_metadata)
		{
			big_aabb.min.x = rsl::min(big_aabb.min.x, metadata.aabb.min.x);
			big_aabb.min.y = rsl::min(big_aabb.min.y, metadata.aabb.min.y);
			big_aabb.max.x = rsl::max(big_aabb.max.x, metadata.aabb.max.x);
			big_aabb.max.y = rsl::max(big_aabb.max.y, metadata.aabb.max.y);
		}

		// Now go over all the minmax results and convert their coordinates
		for (auto& [name, metadata] : map_to_metadata)
		{
			metadata.aabb.min.x -= big_aabb.min.x;
			metadata.aabb.min.y -= big_aabb.min.y;
			metadata.aabb.max.x -= big_aabb.min.x;
			metadata.aabb.max.y -= big_aabb.min.y;
		}

		// Create the tilemap and fill in the tile values of each map
		s32 width = big_aabb.max.x - big_aabb.min.x;
		s32 height = big_aabb.max.y - big_aabb.min.y;
		m_tilemap = rsl::make_unique<rex::Tilemap>(width, height);

		for (const auto& [map, metadata] : map_to_metadata)
		{
			s32 map_width = metadata.aabb.max.x - metadata.aabb.min.x;
			s32 map_height = metadata.aabb.max.y - metadata.aabb.min.y;

			rsl::pointi32 pos = metadata.aabb.min;
			for (s32 row_idx = 0; row_idx < map_height; ++row_idx)
			{
				const u8* row_tiles = &map->tiles()[row_idx * map_width];
				m_tilemap->set(row_tiles, map_width, pos.x + (row_idx * width));
			}
		}

		return map_to_metadata;
	}

	rsl::pointi32 MainEditorWidget::map_pos(const rex::Map* map)
	{
		return m_map_to_metadata.at(m_active_map).aabb.min;
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

	void MainEditorWidget::move_camera_to_pos(rsl::pointi32 pos)
	{
		REX_INFO(LogMainEditor, "Moving camera position to ({}, {})", pos.x, pos.y);
	}
}