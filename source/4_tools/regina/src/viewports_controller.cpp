#include "regina/viewports_controller.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "rex_engine/gfx/imgui/imgui_scoped_widget.h"
#include "rex_engine/gfx/imgui/imgui_utils.h"
#include "rex_engine/filesystem/path.h"
#include "rex_engine/gfx/system/shader_library.h"

namespace regina
{
	ViewportsController::ViewportsController(Scene* activeScene)
		: m_active_scene(activeScene)
	{
		//// Initialize the render target early as this needs to be given to the render pass
		//m_render_target = rex::gfx::gal::instance()->create_render_target(desc.width, desc.height, rex::gfx::TextureFormat::Unorm4Srgb);

		//// Initialize a new render pass for the renderer, which will render the tiles of a map
		//// TODO: Implement
		////rex::scratch_string tile_pass = rex::path::join(rex::engine::instance()->engine_root(), "renderpasses", "tilepass.json");
		////TileRenderPassDesc desc = rex::gfx::resource_cache::instance()->load_render_pass(tile_pass);

		//rex::gfx::TilePassDesc renderpass_desc{};
		//renderpass_desc.renderpass_desc.name = "Viewport Tile Render Pass";
		//renderpass_desc.renderpass_desc.framebuffer_desc.emplace_back(m_render_target.get());

		//// This data should be initialized through data, not hardcoded
		//renderpass_desc.renderpass_desc.pso_desc.output_merger.raster_state = rex::gfx::gal::instance()->common_raster_state(rex::gfx::CommonRasterState::DefaultDepth);
		//renderpass_desc.renderpass_desc.pso_desc.shader_pipeline.vs = rex::gfx::shader_lib::instance()->load(rex::path::join(rex::engine::instance()->project_root(), "shaders", "render_tile_vertex.hlsl"), rex::gfx::ShaderType::Vertex);
		//renderpass_desc.renderpass_desc.pso_desc.shader_pipeline.ps = rex::gfx::shader_lib::instance()->load(rex::path::join(rex::engine::instance()->project_root(), "shaders", "render_tile_pixel.hlsl"), rex::gfx::ShaderType::Pixel);
		//renderpass_desc.renderpass_desc.pso_desc.input_layout =
		//{
		//	// Per vertex data
		//	rex::gfx::InputLayoutElementDesc{ rex::gfx::ShaderSemantic::Position, rex::gfx::ShaderArithmeticType::Float2 },
		//	rex::gfx::InputLayoutElementDesc{ rex::gfx::ShaderSemantic::TexCoord, rex::gfx::ShaderArithmeticType::Float2 },
		//};

		//renderpass_desc.backend_tilemap = build_backend_tilemap();

		//m_render_pass = rex::gfx::renderer::instance()->add_render_pass<rex::gfx::TilePass>(tilepass_desc);
	}

	void ViewportsController::add_new_viewport()
	{

	}

	void ViewportsController::update()
	{
		// Send the render commands
		process_input();
		update_renderpass_data();
		draw_imgui();
	}

	void ViewportsController::process_input()
	{
		// Process the input and update the camera accordingly

		// If 'w' is pressed
		// walk speed = 2;
		// camera_pos.y -= speed

	}
	void ViewportsController::update_renderpass_data()
	{
		// All the renderpass needs is the camera position
		// the tilemap is created on startup, we just need to offset what part of the tilemap we're looking at
		//m_render_pass->update_tile_data(m_camera_pos);
	}
	void ViewportsController::draw_imgui()
	{
		ImGuiWindowFlags window_flags{};
		window_flags |= ImGuiWindowFlags_NoTitleBar;

		if (auto widget = rex::imgui::ScopedWidget("Viewport", nullptr, window_flags))
		{
			// Make sure that we hide the tab bar, it doesn't make sense to have it for our viewport
			ImGui::GetCurrentWindow()->WindowClass.DockNodeFlagsOverrideSet |= ImGuiDockNodeFlags_HiddenTabBar;

			// Use the renderpass's render target as image for the viewport
			//const rex::gfx::RenderTarget* render_target = m_render_pass->render_target();
			//rex::imgui::image(render_target);
		}
	}
	TileMap ViewportsController::build_backend_tilemap(const rsl::vector<rex::MapDesc>& maps)
	{
		// The goal of this function is to create a tilemap and fill in each map that's passed in into this map
		// We do these by doing the following
		// 1. Create a world bounding box
		// 2. Create regions within this bounding box where each map is located
		// 3. Filling in the tile of each map into this



		rsl::unordered_map<rsl::string_view, const rex::MapDesc*> name_to_map;
		name_to_map.reserve(maps.size());
		for (const rex::MapDesc& map_desc : maps)
		{
			name_to_map[map_desc.map_header.name] = &map_desc;
		}

		struct MapWithPos
		{
			const rex::MapDesc* desc;
			rsl::pointi32 pos;
		};
		rsl::pointi32 start_pos{};
		std::vector<MapWithPos> open_nodes;
		open_nodes.reserve(maps.size());
		open_nodes.push_back({ &maps.front(), start_pos});
		//rsl::vector<MinMax> map_rects{};

		//std::vector<MapWithPos> closed_nodes;
		//closed_nodes.reserve(maps.size());

		//while (!open_nodes.empty())
		//{
		//	MapWithPos current_node = open_nodes.back();
		//	open_nodes.pop_back();

		//	// 1. calculate bounding box of current map
		//	MinMax map_rect = calc_map_rect(current_node.desc->map_header, start_pos);
		//	map_rects.push_back(map_rect);

		//	// 2. add the map's connections to the open nodes
		//	for (const rex::MapConnection& conn : current_node.desc->connections)
		//	{
		//		if (!name_to_map.contains(conn.map.name))
		//		{
		//			continue;
		//		}

		//		const rex::MapDesc* conn_map = name_to_map[conn.map.name];
		//		if (rsl::find(closed_nodes.cbegin(), closed_nodes.cend(), conn_map) == closed_nodes.cend())
		//		{
		//			const s32 tiles_per_block = 4;
		//			s32 half_width_in_tiles = (current_node.desc->map_header.width_in_blocks * tiles_per_block / 2);
		//			s32 half_height_in_tiles = (current_node.desc->map_header.height_in_blocks * tiles_per_block / 2);
		//			s32 half_conn_width_in_tiles = (conn.map.width_in_blocks * tiles_per_block / 2);
		//			s32 half_conn_height_in_tiles = (conn.map.height_in_blocks * tiles_per_block / 2);
		//			rsl::pointi32 conn_pos;
		//			if (conn.direction == rex::Direction::North)
		//			{
		//				conn_pos = { start_pos.x + conn.offset, start_pos.y + half_height_in_tiles + half_conn_height_in_tiles };
		//			}
		//			else if (conn.direction == rex::Direction::South)
		//			{
		//				conn_pos = { start_pos.x + conn.offset, start_pos.y - half_height_in_tiles - half_conn_height_in_tiles };
		//			}
		//			else if (conn.direction == rex::Direction::East)
		//			{
		//				conn_pos = { start_pos.x + half_width_in_tiles + half_conn_width_in_tiles, start_pos.y + conn.offset };
		//			}
		//			else if (conn.direction == rex::Direction::West)
		//			{
		//				conn_pos = { start_pos.x - half_width_in_tiles - half_conn_width_in_tiles, start_pos.y + conn.offset };
		//			}
		//			open_nodes.push_back({ conn_map, conn_pos });
		//		}
		//	}

		//	// 3. add the map itself to the closed nodes
		//	closed_nodes.push_back(current_node);
		//}

		








		//rsl::vector<const rex::MapDesc*> visited_maps;
		//struct MapAndPosition
		//{
		//	const rex::MapDesc* desc;
		//	rsl::pointi32 pos;
		//};
		//rsl::unordered_map<rsl::string_view, MapAndPosition> map_to_pos;
		//map_to_pos[maps.front().map_header.name] = { &maps.front(), rsl::pointi32{} };
		//for (const rex::MapDesc& map : maps)
		//{
		//	// If the map has already been visited, we don't need to process it again
		//	if (rsl::find(visited_maps.cbegin(), visited_maps.cend(), &map) != visited_maps.cend())
		//	{
		//		return;
		//	}

		//	// Process the map and mark as visited
		//	MinMax res = calc_map_rect(map.map_header, start_pos);
		//	map_rects.push_back(res);

		//	const s32 tiles_per_block = 4;
		//	s32 half_width_in_tiles = (map.map_header.width_in_blocks * tiles_per_block / 2);
		//	s32 half_height_in_tiles = (map.map_header.height_in_blocks * tiles_per_block / 2);

		//	// Go over each connection of the map and process their rects as well
		//	// Do this recurisvely until all connections are processed
		//	for (const rex::MapConnection& conn : map.connections)
		//	{
		//		s32 half_conn_width_in_tiles = (conn.map.width_in_blocks * tiles_per_block / 2);
		//		s32 half_conn_height_in_tiles = (conn.map.height_in_blocks * tiles_per_block / 2);

		//		rsl::pointi32 conn_pos;
		//		if (conn.direction == rex::Direction::North)
		//		{
		//			conn_pos = { start_pos.x + conn.offset, start_pos.y + half_height_in_tiles + half_conn_height_in_tiles };
		//		}
		//		else if (conn.direction == rex::Direction::South)
		//		{
		//			conn_pos = { start_pos.x + conn.offset, start_pos.y - half_height_in_tiles - half_conn_height_in_tiles };
		//		}
		//		else if (conn.direction == rex::Direction::East)
		//		{
		//			conn_pos = { start_pos.x + half_width_in_tiles + half_conn_width_in_tiles, start_pos.y + conn.offset };
		//		}
		//		else if (conn.direction == rex::Direction::West)
		//		{
		//			conn_pos = { start_pos.x - half_width_in_tiles - half_conn_width_in_tiles, start_pos.y + conn.offset };
		//		}

		//		auto it = rsl::find(maps.cbegin(), maps.cend(), [&](const rex::MapDesc& mapDesc) { return mapDesc.map_header.name == conn.map.name; });
		//		map_to_pos[conn.map.name] = { rsl::iterator_to_pointer(it), conn_pos };
		//	}
		//}

		rsl::pointi32 min = { rsl::numeric_limits<s32>::max() , rsl::numeric_limits<s32>::max() };
		rsl::pointi32 max = { rsl::numeric_limits<s32>::min() , rsl::numeric_limits<s32>::min() };

		//for (MinMax minmax : map_rects)
		//{
		//	min.x = rsl::min(min.x, minmax.min.x);
		//	min.y = rsl::min(min.y, minmax.min.y);

		//	max.x = rsl::max(max.x, minmax.max.x);
		//	max.y = rsl::max(max.y, minmax.max.y);
		//}

		s32 width = max.x - min.x;
		s32 height = max.y - min.y;

		// allocate the memory and fill in the tile indices..

		return TileMap(width, height);
	}
	//void ViewportsController::calc_map_rects(const rex::MapDesc& map, rsl::pointi32 startPos, rsl::vector<const rex::MapDesc*>& visitedMaps, rsl::vector<MinMax>& mapRects)
	//{
	//	// If the map has already been visited, we don't need to process it again
	//	//if (rsl::find(visitedMaps.cbegin(), visitedMaps.cend(), &map) != visitedMaps.cend())
	//	//{
	//	//	return;
	//	//}

	//	//// Process the map and mark as visited
	//	//MinMax res = calc_map_rect(map.map_header, startPos);
	//	//mapRects.push_back(res);

	//	//const s32 tiles_per_block = 4;
	//	//s32 half_width_in_tiles = (map.map_header.width_in_blocks * tiles_per_block / 2);
	//	//s32 half_height_in_tiles = (map.map_header.height_in_blocks * tiles_per_block / 2);

	//	//// Go over each connection of the map and process their rects as well
	//	//// Do this recurisvely until all connections are processed
	//	//for (const rex::MapConnection& conn : map.connections)
	//	//{
	//	//	s32 half_conn_width_in_tiles = (conn.map.width_in_blocks * tiles_per_block / 2);
	//	//	s32 half_conn_height_in_tiles = (conn.map.height_in_blocks * tiles_per_block / 2);

	//	//	rsl::pointi32 conn_pos;
	//	//	if (conn.direction == rex::Direction::North)
	//	//	{
	//	//		conn_pos = { startPos.x + conn.offset, startPos.y + half_height_in_tiles + half_conn_height_in_tiles};
	//	//	}
	//	//	else if (conn.direction == rex::Direction::South)
	//	//	{
	//	//		conn_pos = { startPos.x + conn.offset, startPos.y - half_height_in_tiles - half_conn_height_in_tiles };
	//	//	}
	//	//	else if (conn.direction == rex::Direction::East)
	//	//	{
	//	//		conn_pos = { startPos.x + half_width_in_tiles + half_conn_width_in_tiles, startPos.y + conn.offset };
	//	//	}
	//	//	else if (conn.direction == rex::Direction::West)
	//	//	{
	//	//		conn_pos = { startPos.x - half_width_in_tiles - half_conn_width_in_tiles, startPos.y + conn.offset };
	//	//	}

	//	//	calc_map_rects(conn.map, conn_pos, visitedMaps, mapRects);
	//	//}
	//}
	//MinMax ViewportsController::calc_map_rect(const rex::MapHeader& map, rsl::pointi32 startPos)
	//{
	//	const s32 tiles_per_block = 4;

	//	s32 half_width_in_tiles = (map.width_in_blocks * tiles_per_block / 2);
	//	s32 half_height_in_tiles = (map.height_in_blocks * tiles_per_block / 2);

	//	MinMax res{};
	//	res.min.x = -half_width_in_tiles + startPos.x;
	//	res.min.y = -half_height_in_tiles + startPos.y;
	//	res.max.x =  half_width_in_tiles + startPos.x;
	//	res.max.y =  half_height_in_tiles + startPos.y;

	//	return res;
	//}

}