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
		// This min and max is represented in tiles
		MinMax tilemap_bounding_box{};
		//tilemap_bounding_box.min.x = -120;
		//tilemap_bounding_box.min.y = -234;
		//tilemap_bounding_box.max.x = 480;
		//tilemap_bounding_box.max.y = 492;

		//// Final results
		//tilemap_bounding_box.min.x = 0;
		//tilemap_bounding_box.min.y = 0;
		//tilemap_bounding_box.max.y = 680;
		//tilemap_bounding_box.max.y = 720;

		rsl::vector<MinMax> min_max;
		if (!is_map_in_tilemap(&m_active_map->desc()))
		{
			load_maps();
			min_max = build_tilemap();
			rsl::vector<MinMax> expected_results =
			{
				MinMax{ rsl::string("CELADON CITY"), {180 ,198 }, {280 ,270 } },
				MinMax{ rsl::string("CERULEAN CITY"), {320 ,342 }, {400 ,414 } },
				MinMax{ rsl::string("CINNABAR ISLAND"), {-20 ,-234 }, {20 ,-198 } },
				MinMax{ rsl::string("FUCHSIA CITY"), {200 ,-126 }, {280 ,-54 } },
				MinMax{ rsl::string("INDIGO PLATEAU"), {-116 ,428 }, {-76 ,464 } },
				MinMax{ rsl::string("LAVENDER TOWN"), {520 ,218 }, {560 ,254 } },
				MinMax{ rsl::string("PALLET TOWN"), {-20 ,-18 }, {20 ,18 } },
				MinMax{ rsl::string("PEWTER CITY"), {-40 ,306 }, {40 ,378 } },
				MinMax{ rsl::string("ROUTE 1"), {-20 ,18 }, {20 ,90 } },
				MinMax{ rsl::string("ROUTE 10"), {520 ,254 }, {560 ,398 } },
				MinMax{ rsl::string("ROUTE 11"), {400 ,74 }, {520 ,110 } },
				MinMax{ rsl::string("ROUTE 12"), {520 ,2 }, {560 ,218 } },
				MinMax{ rsl::string("ROUTE 13"), {440 ,-34 }, {560 ,2 } },
				MinMax{ rsl::string("ROUTE 14"), {400 ,-106 }, {440 ,2 } },
				MinMax{ rsl::string("ROUTE 15"), {280 ,-106 }, {400 ,-70 } },
				MinMax{ rsl::string("ROUTE 16"), {100 ,218 }, {180 ,254 } },
				MinMax{ rsl::string("ROUTE 17"), {100 ,-70 }, {140 ,218 } },
				MinMax{ rsl::string("ROUTE 18"), {100 ,-106 }, {200 ,-70 } },
				MinMax{ rsl::string("ROUTE 2"), {-20 ,162 }, {20 ,306 } },
				MinMax{ rsl::string("ROUTE 22"), {-120 ,104 }, {-40 ,140 } },
				MinMax{ rsl::string("ROUTE 23"), {-116 ,140 }, {-76 ,428 } },
				MinMax{ rsl::string("ROUTE 24"), {340 ,414 }, {380 ,486 } },
				MinMax{ rsl::string("ROUTE 25"), {380 ,450 }, {500 ,486 } },
				MinMax{ rsl::string("ROUTE 3"), {40 ,326 }, {180 ,362 } },
				MinMax{ rsl::string("ROUTE 4"), {140 ,362 }, {320 ,398 } },
				MinMax{ rsl::string("ROUTE 5"), {340 ,270 }, {380 ,342 } },
				MinMax{ rsl::string("ROUTE 6"), {340 ,126 }, {380 ,198 } },
				MinMax{ rsl::string("ROUTE 7"), {280 ,218 }, {320 ,254 } },
				MinMax{ rsl::string("ROUTE 8"), {400 ,218 }, {520 ,254 } },
				MinMax{ rsl::string("ROUTE 9"), {400 ,362 }, {520 ,398 } },
				MinMax{ rsl::string("SAFFRON CITY"), {320 ,198 }, {400 ,270 } },
				MinMax{ rsl::string("SEA ROUTE 19"), {220 ,-234 }, {260 ,-126 } },
				MinMax{ rsl::string("SEA ROUTE 20"), {20 ,-234 }, {220 ,-198 } },
				MinMax{ rsl::string("SEA ROUTE 21"), {-20 ,-198 }, {20 ,-18 } },
				MinMax{ rsl::string("VERMILION CITY"), {320 ,54 }, {400 ,126 } },
				MinMax{ rsl::string("VIRIDIAN CITY"), {-40 ,90 }, {40 ,162 } },
			};

			for (MinMax minmax : min_max)
			{
				auto it = std::find_if(expected_results.cbegin(), expected_results.cend(), [&](const MinMax& minmax2) { return minmax2.name == minmax.name; });
				if (it->min != minmax.min)
				{
					REX_INFO(LogMainEditor, "min differs for {}", minmax.name_with_conn);
					REX_INFO(LogMainEditor, "expects: ({}, {})", it->min.x, it->min.y);
					REX_INFO(LogMainEditor, "got: ({}, {})", minmax.min.x, minmax.min.y);
					REX_INFO(LogMainEditor, "diff: ({}, {})", minmax.min.x - it->min.x, minmax.min.y - it->min.y);
				}
				if (it->max != minmax.max)
				{
					REX_INFO(LogMainEditor, "max differs for {}", minmax.name_with_conn);
					REX_INFO(LogMainEditor, "expects: ({}, {})", it->max.x, it->max.y);
					REX_INFO(LogMainEditor, "got: ({}, {})", minmax.max.x, minmax.max.y);
					REX_INFO(LogMainEditor, "diff: ({}, {})", minmax.max.x - it->max.x, minmax.max.y - it->max.y);
				}
			}

			for (MinMax minmax : expected_results)
			{
				tilemap_bounding_box.min.x = rsl::min(tilemap_bounding_box.min.x, minmax.min.x);
				tilemap_bounding_box.min.y = rsl::min(tilemap_bounding_box.min.y, minmax.min.y);
				tilemap_bounding_box.max.x = rsl::max(tilemap_bounding_box.max.x, minmax.max.x);
				tilemap_bounding_box.max.y = rsl::max(tilemap_bounding_box.max.y, minmax.max.y);
			}
		}

		// Expected results
		// { name = "CELADON CITY" min = { x = 180 y = 198 } max = { x = 280 y = 270 } }	regina::MinMax
		// { name = "CERULEAN CITY" min = {x = 320 y = 342 } max = {x = 400 y = 414 } }	regina::MinMax
		// { name = "CINNABAR ISLAND" min = {x = -20 y = -234 } max = {x = 20 y = -198 } }	regina::MinMax
		// { name = "FUCHSIA CITY" min = {x = 200 y = -128 } max = {x = 280 y = -58 } }	regina::MinMax
		// { name = "INDIGO PLATEAU" min = {x = -118 y = 434 } max = {x = -78 y = 470 } }	regina::MinMax
		// { name = "LAVENDER TOWN" min = {x = 520 y = 218 } max = {x = 560 y = 254 } }	regina::MinMax
		// { name = "PALLET TOWN" min = {x = -20 y = -18 } max = {x = 20 y = 18 } }	regina::MinMax
		// { name = "PEWTER CITY" min = {x = -40 y = 306 } max = {x = 40 y = 378 } }	regina::MinMax
		// { name = "ROUTE 1" min = {x = -20 y = 18 } max = {x = 20 y = 90 } }	regina::MinMax
		// { name = "ROUTE 10" min = {x = 520 y = 254 } max = {x = 560 y = 398 } }	regina::MinMax
		// { name = "ROUTE 11" min = {x = 400 y = 74 } max = {x = 520 y = 108 } }	regina::MinMax
		// { name = "ROUTE 12" min = {x = 520 y = 2 } max = {x = 560 y = 218 } }	regina::MinMax
		// { name = "ROUTE 13" min = {x = 440 y = -34 } max = {x = 560 y = 2 } }	regina::MinMax
		// { name = "ROUTE 14" min = {x = 400 y = -104 } max = {x = 440 y = 2 } }	regina::MinMax
		// { name = "ROUTE 15" min = {x = 280 y = -104 } max = {x = 400 y = -68 } }	regina::MinMax
		// { name = "ROUTE 16" min = {x = 100 y = 218 } max = {x = 180 y = 254 } }	regina::MinMax
		// { name = "ROUTE 17" min = {x = 100 y = -68 } max = {x = 140 y = -16 } }	regina::MinMax
		// { name = "ROUTE 18" min = {x = 100 y = -104 } max = {x = 194 y = -68 } }	regina::MinMax
		// { name = "ROUTE 2" min = {x = -20 y = 162 } max = {x = 20 y = 306 } }	regina::MinMax
		// { name = "ROUTE 22" min = {x = -120 y = 104 } max = {x = -40 y = 140 } }	regina::MinMax
		// { name = "ROUTE 23" min = {x = -116 y = 140 } max = {x = -96 y = 428 } }	regina::MinMax
		// { name = "ROUTE 24" min = {x = 340 y = 414 } max = {x = 380 y = 486 } }	regina::MinMax
		// { name = "ROUTE 25" min = {x = 380 y = 450 } max = {x = 500 y = 486 } }	regina::MinMax
		// { name = "ROUTE 3" min = {x = 40 y = 326 } max = {x = 180 y = 362 } }	regina::MinMax
		// { name = "ROUTE 4" min = {x = 140 y = 362 } max = {x = 320 y = 398 } }	regina::MinMax
		// { name = "ROUTE 5" min = {x = 340 y = 270 } max = {x = 380 y = 342 } }	regina::MinMax
		// { name = "ROUTE 6" min = {x = 340 y = 126 } max = {x = 380 y = 198 } }	regina::MinMax
		// { name = "ROUTE 7" min = {x = 280 y = 28 } max = {x = 320 y = 254 } }	regina::MinMax
		// { name = "ROUTE 8" min = {x = 400 y = 218 } max = {x = 520 y = 254 } }	regina::MinMax
		// { name = "ROUTE 9" min = {x = 400 y = 362 } max = {x = 520 y = 398 } }	regina::MinMax
		// { name = "SAFFRON CITY" min = {x = 320 y = 198 } max = {x = 400 y = 270 } }	regina::MinMax
		// { name = "SEA ROUTE 19" min = {x = 220 y = -234 } max = {x = 260 y = -180 } }	regina::MinMax
		// { name = "SEA ROUTE 20" min = {x = -10 y = -234 } max = {x = 190 y = -198 } }	regina::MinMax
		// { name = "SEA ROUTE 21" min = {x = -20 y = -198 } max = {x = 20 y = -18 } }	regina::MinMax
		// { name = "VERMILION CITY" min = {x = 320 y = 43 } max = {x = 400 y = 115 } }	regina::MinMax
		// { name = "VIRIDIAN CITY" min = {x = -40 y = 90 } max = {x = 40 y = 162 } }	regina::MinMax

		tilemap_bounding_box.max.x -= tilemap_bounding_box.min.x;
		tilemap_bounding_box.min.x -= tilemap_bounding_box.min.x;

		tilemap_bounding_box.max.y -= tilemap_bounding_box.min.y;
		tilemap_bounding_box.min.y -= tilemap_bounding_box.min.y;


		rsl::pointi32 pos_in_tilemap = find_map_pos_in_tilemap(&m_active_map->desc());
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
			if (rsl::find(closed_nodes.cbegin(), closed_nodes.cend(), current_node) != closed_nodes.cend())
			{
				continue;
			}

			MapJson map = load_map(current_node);
			for (const MapConnectionJson& conn : map.connections)
			{
				open_nodes.push_back(rsl::string(rex::path::join(rex::engine::instance()->data_root(), conn.map)));
			}

			m_map_jsons.push_back(rsl::move(map));
			m_maps.push_back(rex::asset_db::instance()->load_from_json<rex::Map>(current_node));
			closed_nodes.push_back(current_node);
		}
	}

	rsl::vector<MinMax> MainEditorWidget::build_tilemap()
	{
		struct MapWithPos
		{
			const rex::MapDesc* desc;
			rsl::pointi32 pos;
			const rex::MapDesc* parent;
		};
		rsl::pointi32 start_pos{};
		std::vector<MapWithPos> open_nodes;

		open_nodes.push_back({ &m_active_map->desc(), start_pos, nullptr});
		rsl::vector<MinMax> map_rects{};

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
			if (current_node.parent)
			{
				map_rect.name_with_conn = rsl::string(rsl::format("{} -> {}", current_node.parent->map_header.name, current_node.desc->map_header.name));
			}
			map_rects.push_back(map_rect);

			// 2. add the map itself to the closed nodes
			closed_nodes.push_back(current_node);

			// 3. add the map's connections to the open nodes
			for (const rex::MapConnection& conn : current_node.desc->connections)
			{
				auto closed_node_it = std::find_if(closed_nodes.cbegin(), closed_nodes.cend(), [&](const MapWithPos& mapWithPos) 
					{
						return mapWithPos.desc->map_header.name == conn.map.name;
					});
				if (closed_node_it != closed_nodes.cend())
				{
					continue;
				}

				auto map_desc_it = std::find_if(m_maps.cbegin(), m_maps.cend(), [&](rex::Map* map) 
					{
						return map->desc().map_header.name == conn.map.name;
					});

				const rex::MapDesc* conn_map = &(*map_desc_it)->desc();
				if (rsl::find_if(closed_nodes.cbegin(), closed_nodes.cend(), [&](const MapWithPos& mapWithPos) { return mapWithPos.desc == conn_map; }) == closed_nodes.cend())
				{
					const s32 tiles_per_block = 4;
					s32 half_width_in_tiles = (current_node.desc->map_header.width_in_blocks * tiles_per_block / 2);
					s32 half_height_in_tiles = (current_node.desc->map_header.height_in_blocks * tiles_per_block / 2);
					s32 half_conn_width_in_tiles = (conn.map.width_in_blocks * tiles_per_block / 2);
					s32 half_conn_height_in_tiles = (conn.map.height_in_blocks * tiles_per_block / 2);
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
					open_nodes.push_back({ conn_map, conn_pos, current_node.desc });
				}
			}
		}

		return map_rects;
	}

	MinMax MainEditorWidget::calc_map_rect(const rex::MapHeader& map, rsl::pointi32 startPos)
	{
		const s32 tiles_per_block = 4;

		s32 half_width_in_tiles = (map.width_in_blocks * tiles_per_block / 2);
		s32 half_height_in_tiles = (map.height_in_blocks * tiles_per_block / 2);

		MinMax res{};
		res.name = map.name;
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

	rsl::pointi32 MainEditorWidget::find_map_pos_in_tilemap(const rex::MapDesc* map)
	{
		return {};
	}
	void MainEditorWidget::move_camera_to_pos(rsl::pointi32 pos)
	{

	}
}