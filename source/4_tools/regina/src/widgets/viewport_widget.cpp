#include "regina/widgets/viewport_widget.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

#include "rex_engine/gfx/graphics.h"
#include "rex_engine/gfx/imgui/imgui_scoped_widget.h"
#include "rex_engine/gfx/imgui/imgui_utils.h"

#include "rex_engine/engine/asset_db.h"

#include "rex_engine/filesystem/path.h"
#include "rex_std/bonus/math.h"

#include "rex_engine/gfx/rendering/renderer.h"
#include "rex_engine/gfx/resources/sampler_2d.h"
#include "rex_engine/gfx/system/resource_cache.h"
#include "rex_engine/gfx/system/shader_library.h"

#include "rex_engine/shapes/rect.h"

#include "rex_engine/assets/blockset.h"
#include "rex_engine/assets/map.h"

namespace regina
{
	namespace constants
	{
		constexpr u8 g_map_padding_blocks = 3;

		constexpr u8 g_num_tiles_per_square_row = 2;
		constexpr u8 g_num_tiles_per_square_column = 2;
		constexpr u8 g_num_tiles_per_square = g_num_tiles_per_square_row * g_num_tiles_per_square_column;

		constexpr u8 g_num_tiles_per_block_row = 4;
		constexpr u8 g_num_tiles_per_block_column = 4;
		constexpr u8 g_num_tiles_per_block = g_num_tiles_per_block_row * g_num_tiles_per_block_column;

		constexpr u8 g_tile_width_px = 8;
		constexpr u8 g_tile_height_px = 8;
		constexpr u8 g_tile_size_px = g_tile_width_px * g_tile_height_px;

		constexpr u8 g_square_width_px = g_num_tiles_per_square_row * g_tile_width_px;
		constexpr u8 g_square_height_px = g_num_tiles_per_square_column * g_tile_height_px;
		constexpr u16 g_square_size_px = g_square_width_px * g_square_height_px;

		constexpr u8 g_block_width_px = g_num_tiles_per_block_row * g_tile_width_px;
		constexpr u8 g_block_height_px = g_num_tiles_per_block_column * g_tile_height_px;
		constexpr u16 g_block_size_px = g_block_width_px * g_block_height_px;

		constexpr u8 g_screen_width = 160;
		constexpr u8 g_screen_height = 144;

		constexpr u8 g_screen_width_in_tiles = g_screen_width / g_tile_width_px;
		constexpr u8 g_screen_height_in_tiles = g_screen_height / g_tile_height_px;

		constexpr u8 g_screen_width_in_squares = g_screen_width / g_square_width_px;
		constexpr u8 g_screen_height_in_squares = g_screen_height / g_square_height_px;

		constexpr u8 g_screen_width_in_blocks = g_screen_width / g_block_width_px;
		constexpr u8 g_screen_height_in_blocks = g_screen_height / g_block_height_px;

		constexpr rsl::pointi8 player_render_position_top_left = { 8,8 };
		constexpr rsl::pointi8 player_render_position_bottom_right =
		{
			g_screen_width_in_tiles - player_render_position_top_left.x,
			g_screen_height_in_tiles - player_render_position_top_left.y
		};
	}

	struct TileVertex
	{
		rsl::point<f32> pos;
		rsl::point<f32> uv;
	};

	// This struct is the mapping of the constant buffer with the same name in the shader
	struct TileRenderingMetaData
	{
		// Tile texture data
		u32 texture_tiles_per_row;   // the number of tiles per row in the tileset texture
		f32 inv_texture_width;       // the inverse width of the tileset texture, in pixels
		f32 inv_texture_height;      // the inverse height of the tileset texture, in pixels

		// Render target data
		u32 screen_tiles_per_row;    // the number of tiles we render on a single row
		f32 inv_tile_screen_width;   // the inverse of the width of a single tile on the screen
		f32 inv_tile_screen_height;  // the inverse of the height of a single tile on the screen
	};

	struct TileRenderPassDesc
	{
		rex::gfx::RenderPassDesc renderpass_desc;
		TilesetDesc tileset_desc;
		const rex::Map* map;
	};

	// Returns the rect in which the connection map should be drawn
	rex::Rect8 rect_for_connection(const rex::Map* map, const rex::MapConnection& conn)
	{
		// Maps are always center aligned, their midpoint shifts depending on the connection offset
		s8 mid_point_x = constants::g_map_padding_blocks + ((map->width_in_blocks() / 2) + conn.offset);
		s8 mid_point_y = constants::g_map_padding_blocks + ((map->height_in_blocks() / 2) + conn.offset);

		s8 start = 0;
		s8 end = 0;
		
		// The formula to calculate the start is the same for each direction, just takes different input
		// depending on the connection's direction
		if (conn.direction == rex::Direction::North || conn.direction == rex::Direction::South)
		{
			s8 conn_half_width_in_blocks = (conn.map.width_in_blocks / 2);
			s8 offsetted_start = mid_point_x - conn_half_width_in_blocks + conn.offset;
			s32 max_end = map->width_in_blocks() + constants::g_map_padding_blocks;

			start = static_cast<s8>(rsl::clamp_min(offsetted_start, 0));
			end = rex::narrow_cast<s8>(rsl::min(max_end, start + conn.map.width_in_blocks));
		}
		else
		{
			s8 conn_half_height_in_blocks = (conn.map.height_in_blocks / 2);
			s8 offsetted_start = mid_point_y - conn_half_height_in_blocks + conn.offset;
			s32 max_end = map->height_in_blocks() + constants::g_map_padding_blocks;

			start = static_cast<s8>(rsl::clamp_min(offsetted_start, 0));
			end = rex::narrow_cast<s8>(rsl::min(max_end, start + conn.map.height_in_blocks));
		}

		// Fill in the rex::Rect8 based on the direction of the connection
		rex::Rect8 res{};

		switch (conn.direction)
		{
		case rex::Direction::North:
			res.top_left.x = start;
			res.top_left.y = 0;
			res.bottom_right.x = end;
			res.bottom_right.y = res.top_left.y + constants::g_map_padding_blocks;
			break;
		case rex::Direction::East:
			res.top_left.x = map->width_in_blocks() + constants::g_map_padding_blocks;
			res.top_left.y = start;
			res.bottom_right.x = res.top_left.x + constants::g_map_padding_blocks;
			res.bottom_right.y = end;
			break;
		case rex::Direction::South:
			res.top_left.x = start;
			res.top_left.y = map->height_in_blocks() + constants::g_map_padding_blocks;
			res.bottom_right.x = end;
			res.bottom_right.y = res.top_left.y + constants::g_map_padding_blocks;
			break;
		case rex::Direction::West:
			res.top_left.x = 0;
			res.top_left.y = start;
			res.bottom_right.x = res.top_left.x + constants::g_map_padding_blocks;
			res.bottom_right.y = end;
			break;
		}

		return res;
	}

	// Given a point in a map matrix, determine the coordinate of this point in the connecting map
	// let's say you have a map connecting to another in the north, both having an equal width and no offset
	// then a given coordinate {0, 0}, would return {0, height - constants::g_map_padding_blocks}
	rsl::pointi8 project_point_to_conn(const rex::Map* map, const rex::MapConnection& conn, rsl::pointi8 coord)
	{
		s8 projected_point = 0;
		if (conn.direction == rex::Direction::North || conn.direction == rex::Direction::South)
		{
			s8 mid_point_x = conn.map.width_in_blocks / 2 - conn.offset;
			s8 offset_from_mid = constants::g_map_padding_blocks + (map->width_in_blocks() / 2) - coord.x;
			projected_point = mid_point_x - offset_from_mid;
		}
		else
		{
			s8 mid_point_y = conn.map.height_in_blocks / 2 - conn.offset;
			s8 offset_from_mid = constants::g_map_padding_blocks + (map->height_in_blocks() / 2) - coord.y;
			projected_point = mid_point_y - offset_from_mid;
		}

		rsl::pointi8 res{};

		switch (conn.direction)
		{
		case rex::Direction::North:
			res.x = projected_point;
			res.y = conn.map.height_in_blocks - constants::g_map_padding_blocks + coord.y;
			break;
		case rex::Direction::East:
			res.x = coord.x - constants::g_map_padding_blocks + map->width_in_blocks();
			res.y = projected_point;
			break;
		case rex::Direction::South:
			res.x = projected_point;
			res.y = coord.y - constants::g_map_padding_blocks + map->height_in_blocks();
			break;
		case rex::Direction::West:
			res.x = conn.map.width_in_blocks - constants::g_map_padding_blocks + coord.x;
			res.y = projected_point;
			break;
		}

		return res;
	}

	void MapMatrix::build(const rex::Map* map)
	{
		m_total_width = map->width_in_blocks() + (2 * constants::g_map_padding_blocks);
		m_total_height = map->height_in_blocks() + (2 * constants::g_map_padding_blocks);

		m_block_indices = rsl::make_unique<u8[]>(m_total_width * m_total_height);
		init_blocks(map->border_block_idx());

		fill_connections(map);
		fill_inner_map(map);
	}

	s32 MapMatrix::width_in_blocks() const
	{
		return m_total_width;
	}
	s32 MapMatrix::height_in_blocks() const
	{
		return m_total_height;
	}
	s32 MapMatrix::major_dimension_in_blocks() const
	{
		return width_in_blocks();
	}
	s32 MapMatrix::width_in_tiles() const
	{
		return width_in_blocks() * constants::g_num_tiles_per_block_row;
	}
	s32 MapMatrix::height_in_tiles() const
	{
		return height_in_blocks() * constants::g_num_tiles_per_block_column;
	}
	s32 MapMatrix::major_dimension_in_tiles() const
	{
		return width_in_tiles();
	}

	s32 MapMatrix::index_at(rsl::pointi8 blockCoord) const
	{
		s32 idx = coords::coord_to_index(blockCoord, m_total_width);
		return m_block_indices[idx];
	}
	s32 MapMatrix::index_at(rsl::pointi8 tileCoord) const
	{
		rsl::pointi8 block_coord = coords::tile_to_block_coord(tileCoord);
		return index_at(block_coord);
	}

	void MapMatrix::init_blocks(s8 borderBlockIdx)
	{
		rsl::fill_n(m_block_indices.begin(), m_block_indices.count(), borderBlockIdx);
	}
	void MapMatrix::fill_connections(const rex::Map* map)
	{
		for (const rex::MapConnection& conn : map->connections())
		{
			// Calculate the rect in which the connection will be drawn
			// This rect covers the entire padding region
			rex::Rect8 rect = rect_for_connection(map, conn);

			// Calculate the top left position where the drawing will start, inside of this rect
			rsl::pointi8 top_left_conn = project_point_to_conn(map, conn, rect.top_left);

			// Load the map blocks of the connection so we can assign the right block index to the map matrix
			const rsl::unique_array<u8>& conn_map_blocks = asset_db::find_map_blocks(conn.map->map_header);

			// Go over the blocks of the connection and assign the block index to the map matrix
			for (s8 y = rect.top_left.y, conn_y = top_left_conn.y; y < rect.bottom_right.y; ++y, ++conn_y)
			{
				for (s8 x = rect.top_left.x, conn_x = top_left_conn.x; x < rect.bottom_right.x; ++x, ++conn_x)
				{
					// Look up the block index of the current block we're looking at in the connection map
					s16 conn_idx = conn_y * conn.map.width_in_blocks + conn_x;
					u8 block_idx = conn_map_blocks[conn_idx];

					// Assign the connection's block index to the map matrix
					s16 index = (y * m_total_width) + x;
					m_block_indices[index] = block_idx;
				}
			}
		}
	}
	void MapMatrix::fill_inner_map(const rex::Map* map)
	{
		s32 height = map->height_in_blocks();
		s32 width = map->width_in_blocks();

		const rsl::unique_array<u8>& map_blocks = asset_db::find_map_blocks(map->map_header);
		for (s8 y = 0; y < height; ++y)
		{
			for (s8 x = 0; x < width; ++x)
			{
				s32 index = y * width + x;
				s16 map_matrix_index = ((y + constants::g_map_padding_blocks) * m_total_width) + (x + constants::g_map_padding_blocks);
				m_block_indices[map_matrix_index] = map_blocks[index++];
			}
		}
	}

	// This represents the renderpass that's used to render tiles
	// A 2D game often uses tiles instead of meshes
	// The tilepass only renders the tiles that are in view.
	// It starts in the top left and assigns tile indices row by row
	// The resulting matrix is send to the GPU, which uses these indices
	// to draw the tile at the correct location
	class TilePass : public rex::gfx::RenderPass
	{
	public:
		TilePass(const TileRenderPassDesc& desc)
			: rex::gfx::RenderPass(desc.renderpass_desc)
			, m_tile_width_px(desc.tileset_desc.tile_width_px)
			, m_tile_height_px(desc.tileset_desc.tile_height_px)
			, m_tileset_texture(desc.tileset_desc.tileset_texture->texture_resource())
			, m_blockset(desc.tileset_desc.blockset)
		{
			REX_ASSERT_X(desc.renderpass_desc.framebuffer_desc.size() == 1, "You can only have 1 render target output for a tile render pass");
			REX_ASSERT_X(desc.renderpass_desc.framebuffer_desc.front().use_swapchain() == false, "A tile render pass cannot directly render to the swapchain");

			init(desc);
		}

		// Update the camera where to render from
		void update_camera(rsl::pointi8 cameraPos)
		{
			m_camera_pos = cameraPos;
		}
		// Reserve an uninitialized list of tiles to render
		void reserve_objects()
		{}
		// Push a new tile to render
		void push_object()
		{}
		// Return the render target that's used as output for this pass
		const rex::gfx::RenderTarget* render_target() const
		{
			// Return the first render target of the base class
			// We should only have 1
			return rex::gfx::RenderPass::render_target(0);
		}

		// Update the tileset texture that's used to render the tiles
		void update_tileset_texture(const rex::gfx::Texture2D* texture)
		{}
		void update_dimension(s32 width, s32 height)
		{}

		void pre_pass() override 
		{
			update_tile_data(m_camera_pos);

			upload_tile_data_to_gpu();
		}
		void run_pass() override
		{
			auto render_ctx = rex::gfx::gal::instance()->new_render_ctx();

			// Bind data to the render pipeline
			bind_to(render_ctx.get());
			render_ctx->set_vertex_buffer(m_tiles_vb_gpu.get());
			render_ctx->set_index_buffer(m_tiles_ib_gpu.get());

			// Perform the draw calls
			const s32 index_count_per_instance = 6;
			const s32 instance_count = num_tiles();
			render_ctx->draw_indexed_instanced(index_count_per_instance, instance_count, 0, 0, 0);
		}
		void post_pass() override
		{}

	private:
		s32 num_tiles() const
		{
			return rendertarget_width_in_tiles() * rendertarget_height_in_tiles();
		}

		void init(const TileRenderPassDesc& desc)
		{
			// The tile cache is like a map matrix but it holds indices on a tile level instead of block level
			m_tile_instance_to_tile_idx_cpu = rsl::make_unique<s8[]>(num_tiles());

			// Initialize all the GPU resources
			init_per_tile_vb();
			init_ib();
			init_cb();
			init_tile_instance_to_tile_idx_buffer();
			init_sampler();
			init_shader_params();
			init_map_matrix(desc);
		}
		void init_per_tile_vb()
		{
			// The goal of this func is to create a vertex buffer that holds the following data
			//
			// v0                          v1
			//  +--------------------------+
			//  |                          |
			//  |                          |
			//  |                          |
			//  |                          |
			//  |                          |
			//  |                          |
			//  |                          |
			//  |                          |
			//  |                          |
			//  +--------------------------+
			// v2                          v3

			// We do this by calculating the screen coordinates (in NDC) of a single tile
			// if it were drawn in the top left
			// The shader is responsible for moving this vertex into the correct location on screen
			// based on the instance ID of the tile being drawn

			// Because NDC coordinates go from -1 to 1, divide 2.0f by the width/height
			f32 inv_tile_screen_width = 2.0f / rendertarget_width_in_tiles();
			f32 inv_tile_screen_height = 2.0f / rendertarget_height_in_tiles();

			f32 uv_width = static_cast<f32>(m_tile_width_px) / m_tileset_texture->width();
			f32 uv_height = static_cast<f32>(m_tile_height_px) / m_tileset_texture->height();

			constexpr s32 num_vertices_per_tile = 4;

			rsl::array<TileVertex, num_vertices_per_tile> tile_vertices{};
			// A single tile starts from the top left and spans the entire screen
			// NDC coordinates for a screen go from -1 to 1, meaning that width and height should be of a unit of 2
			tile_vertices[0] = TileVertex{ rsl::point<f32>(0,                                           0), rsl::point<f32>(0.0f, 0.0f) };            // top left
			tile_vertices[1] = TileVertex{ rsl::point<f32>(inv_tile_screen_width,                       0), rsl::point<f32>(uv_width, 0.0f) };        // top right
			tile_vertices[2] = TileVertex{ rsl::point<f32>(0,                     -inv_tile_screen_height), rsl::point<f32>(0.0f, uv_height) };       // bottom left
			tile_vertices[3] = TileVertex{ rsl::point<f32>(inv_tile_screen_width, -inv_tile_screen_height), rsl::point<f32>(uv_width, uv_height) };   // bottom right

			// It's possible we're updating the tiles vertex buffer due to a map transition (as UV could be different)
			// In which case we don't need to allocate a new one, just update the existing one
			if (!m_tiles_vb_gpu)
			{
				m_tiles_vb_gpu = rex::gfx::gal::instance()->create_vertex_buffer(num_vertices_per_tile, sizeof(TileVertex));
			}

			auto render_ctx = rex::gfx::gal::instance()->new_render_ctx();
			render_ctx->update_buffer(m_tiles_vb_gpu.get(), tile_vertices.data(), tile_vertices.size() * sizeof(TileVertex));
			render_ctx->transition_buffer(m_tiles_vb_gpu.get(), rex::gfx::ResourceState::VertexAndConstantBuffer);
		}
		void init_ib()
		{
			constexpr s32 num_indices_per_tile = 6;
			rsl::unique_array<u16> tiles_ib_cpu = rsl::make_unique<u16[]>(num_indices_per_tile);

			// first triangle
			tiles_ib_cpu[0] = 0;
			tiles_ib_cpu[1] = 1;
			tiles_ib_cpu[2] = 2;

			// second triangle
			tiles_ib_cpu[3] = 1;
			tiles_ib_cpu[4] = 3;
			tiles_ib_cpu[5] = 2;

			m_tiles_ib_gpu = rex::gfx::gal::instance()->create_index_buffer(num_indices_per_tile, rex::gfx::IndexBufferFormat::Uint16);

			auto render_ctx = rex::gfx::gal::instance()->new_render_ctx();
			render_ctx->update_buffer(m_tiles_ib_gpu.get(), tiles_ib_cpu.get(), tiles_ib_cpu.byte_size());
			render_ctx->transition_buffer(m_tiles_ib_gpu.get(), rex::gfx::ResourceState::IndexBuffer);
		}
		void init_cb()
		{
			TileRenderingMetaData tile_rendering_meta_data{};
			tile_rendering_meta_data.texture_tiles_per_row = texture_width_in_tiles();
			tile_rendering_meta_data.inv_texture_width = 1.0f / m_tileset_texture->width();
			tile_rendering_meta_data.inv_texture_height = 1.0f / m_tileset_texture->height();

			tile_rendering_meta_data.screen_tiles_per_row = rendertarget_width_in_tiles();
			tile_rendering_meta_data.inv_tile_screen_width = 2.0f / render_target()->width();
			tile_rendering_meta_data.inv_tile_screen_height = 2.0f / render_target()->height();

			// It's possible we're updating and not creating due to a map transition
			// In which case we don't need to allocate a new one, just update the existing one
			if (!m_tile_rendering_meta_data_cb)
			{
				m_tile_rendering_meta_data_cb = rex::gfx::gal::instance()->create_constant_buffer(sizeof(tile_rendering_meta_data));
			}

			auto render_ctx = rex::gfx::gal::instance()->new_render_ctx();
			render_ctx->update_buffer(m_tile_rendering_meta_data_cb.get(), &tile_rendering_meta_data, sizeof(tile_rendering_meta_data));
		}
		void init_tile_instance_to_tile_idx_buffer()
		{
			// We don't have any data for this buffer yet, so we don't upload anything
			m_tile_instance_to_tile_idx_gpu = rex::gfx::gal::instance()->create_unordered_access_buffer(num_tiles());
			auto render_context = rex::gfx::gal::instance()->new_render_ctx();
			render_context->transition_buffer(m_tile_instance_to_tile_idx_gpu.get(), rex::gfx::ResourceState::NonPixelShaderResource);
		}
		void init_sampler()
		{
			m_default_sampler = rex::gfx::gal::instance()->common_sampler(rex::gfx::CommonSampler::Default2D);
		}
		void init_shader_params()
		{
			set("RenderingMetaData", m_tile_rendering_meta_data_cb.get());
			set("tile_texture", m_tileset_texture);
			set("tile_instance_to_tile_idx_in_texture", m_tile_instance_to_tile_idx_gpu.get());

			// This data should be initialized through data, not hardcoded
			set("default_sampler", m_default_sampler);
		}
		void init_map_matrix(const TileRenderPassDesc& desc)
		{
			m_map_matrix.build(desc.map);
		}

		void update_tile_data(const rsl::pointi8 cameraPos)
		{
			// Calculate the first block from which we should start drawing
			rsl::pointi8 top_left = player_pos_to_screen_top_left(m_camera_pos);

			// Start the loop from this block, going left to right, top to down
			// Restricting to only the tiles that'll be rendered
			s32 num_tile_columns = rendertarget_width_in_tiles();
			s32 num_tile_rows = rendertarget_height_in_tiles();
			for (s32 y = 0; y < num_tile_rows; ++y)
			{
				for (s32 x = 0; x < num_tile_columns; ++x)
				{
					// Get the tile coord of the tile we're currently processing
					rsl::pointi8 coord = top_left;
					coord.x += static_cast<s8>(x);
					coord.y += static_cast<s8>(y);

					// Get the block the tile belongs to
					s32 block_idx = m_map_matrix.index_at(coord);
					const rex::Block& block = m_blockset->block(block_idx);

					// Get the tile coordinate of the the first tile in the block (which is top left)
					rsl::pointi8 block_top_left = block_top_left_coord(coord);

					// Get the relative vector from this first tile to the current tile
					// Based on that, calculate which tile in the block we're currently processing
					rsl::pointi8 coord_rel_to_block = coord - block_top_left;
					u8 tile_idx = block.index_at(coord_rel_to_block);

					// Store the tile index in the cache
					s32 tile_instance_idx = y * num_tile_columns + x;
					m_tile_instance_to_tile_idx_cpu[tile_instance_idx] = tile_idx;
				}
			}
		}
		void upload_tile_data_to_gpu()
		{
			auto render_ctx = rex::gfx::gal::instance()->new_render_ctx();
			render_ctx->update_buffer(m_tile_instance_to_tile_idx_gpu.get(), m_tile_instance_to_tile_idx_cpu.get(), m_tile_instance_to_tile_idx_cpu.byte_size());
			render_ctx->transition_buffer(m_tile_instance_to_tile_idx_gpu.get(), rex::gfx::ResourceState::NonPixelShaderResource);
		}

		f32 rendertarget_width_in_tiles() const
		{
			return render_target()->width() / static_cast<f32>(m_tile_width_px);
		}
		f32 rendertarget_height_in_tiles() const
		{
			return render_target()->height() / static_cast<f32>(m_tile_height_px);
		}
		f32 texture_width_in_tiles() const
		{
			return m_tileset_texture->width() / static_cast<f32>(m_tile_width_px);
		}
		f32 texture_height_in_tiles() const
		{
			return m_tileset_texture->height() / static_cast<f32>(m_tile_height_px);
		}

		rsl::pointi8 player_pos_to_screen_top_left(rsl::pointi8 coord)
		{
			// The player is always in tile coord {8, 8};

			rsl::pointi8 screen_top_left = coord;
			screen_top_left.x -= constants::player_render_position_top_left.x;
			screen_top_left.y -= constants::player_render_position_top_left.y;

			return screen_top_left;
		}

		rsl::pointi8 block_to_tile_coord(rsl::pointi8 coord)
		{
			rsl::pointi8 tile_coord{};
			tile_coord.x = coord.x * constants::g_num_tiles_per_block_row;
			tile_coord.y = coord.y * constants::g_num_tiles_per_block_column;

			return tile_coord;
		}
		rsl::pointi8 tile_to_block_coord(rsl::pointi8 coord)
		{
			rsl::pointi8 block_coord{};
			block_coord.x = coord.x / constants::g_num_tiles_per_block_row;
			block_coord.y = coord.y / constants::g_num_tiles_per_block_column;

			return block_coord;
		}
		rsl::pointi8 block_top_left_coord(rsl::pointi8 coord)
		{
			rsl::pointi8 block_coord = tile_to_block_coord(coord);
			return block_to_tile_coord(block_coord);
		}

	private:
		s32 m_tile_width_px;   // the width of a single tile in pixels
		s32 m_tile_height_px;  // the height of a single tile in pixels

		rsl::pointi8 m_camera_pos;
		const rex::Blockset* m_blockset;   // The blockset to use to retrieve the tileset indices from
		MapMatrix m_map_matrix;

		// Gpu resources required for rendering tiles
		rsl::unique_ptr<rex::gfx::VertexBuffer>   m_tiles_vb_gpu;																// This holds the 4 vertices used to make up a plane
		rsl::unique_ptr<rex::gfx::IndexBuffer>    m_tiles_ib_gpu;																// This holds the indices for every tile. This never changes at runtime
		rsl::unique_ptr<rex::gfx::ConstantBuffer> m_tile_rendering_meta_data_cb;								// This holds the extra metadata the shader needs to calculate the UVs for each instance
		rsl::unique_ptr<rex::gfx::UnorderedAccessBuffer> m_tile_instance_to_tile_idx_gpu;				// This holds the mapping of each tile instance to the tile idx in the texture

		const rex::gfx::Texture2D* m_tileset_texture;				// the tileset texture to use
		const rex::gfx::Sampler2D* m_default_sampler;				// The sampler for the tileset texture

		rsl::unique_array<s8> m_tile_instance_to_tile_idx_cpu;
	};

	ViewportWidget::ViewportWidget(const ViewportWidgetDesc& desc)
		// These settings should be loaded from settings
		// As the user will likely want to start from the where they left the camera last time
		: m_camera_pos(desc.camera_pos)
	{
		// Initialize the render target early as this needs to be given to the render pass
		m_render_target = rex::gfx::gal::instance()->create_render_target(desc.width, desc.height, rex::gfx::TextureFormat::Unorm4Srgb);

		// Initialize a new render pass for the renderer, which will render the tiles of a map
		// TODO: Implement
		//rex::scratch_string tile_pass = rex::path::join(rex::engine::instance()->engine_root(), "renderpasses", "tilepass.json");
		//TileRenderPassDesc desc = rex::gfx::resource_cache::instance()->load_render_pass(tile_pass);

		TileRenderPassDesc renderpass_desc;
		renderpass_desc.renderpass_desc.name = "Viewport Tile Render Pass";
		renderpass_desc.renderpass_desc.framebuffer_desc.emplace_back(m_render_target.get());

		// This data should be initialized through data, not hardcoded
		renderpass_desc.renderpass_desc.pso_desc.output_merger.raster_state = rex::gfx::gal::instance()->common_raster_state(rex::gfx::CommonRasterState::DefaultDepth);
		renderpass_desc.renderpass_desc.pso_desc.shader_pipeline.vs = rex::gfx::shader_lib::instance()->load(rex::path::join(rex::engine::instance()->project_root(), "shaders", "render_tile_vertex.hlsl"), rex::gfx::ShaderType::Vertex);
		renderpass_desc.renderpass_desc.pso_desc.shader_pipeline.ps = rex::gfx::shader_lib::instance()->load(rex::path::join(rex::engine::instance()->project_root(), "shaders", "render_tile_pixel.hlsl"), rex::gfx::ShaderType::Pixel);
		renderpass_desc.renderpass_desc.pso_desc.input_layout =
		{
			// Per vertex data
			rex::gfx::InputLayoutElementDesc{ rex::gfx::ShaderSemantic::Position, rex::gfx::ShaderArithmeticType::Float2 },
			rex::gfx::InputLayoutElementDesc{ rex::gfx::ShaderSemantic::TexCoord, rex::gfx::ShaderArithmeticType::Float2 },
		};

		renderpass_desc.tileset_desc = desc.tileset_desc;

		m_render_pass = rex::gfx::renderer::instance()->add_render_pass<TilePass>(desc);
	}

	bool ViewportWidget::on_update()
	{
		process_input();
		update_renderpass_data();
		draw_imgui();

		return false;
	}

	void ViewportWidget::process_input()
	{
		// Process the input and update the camera accordingly
	}
	void ViewportWidget::update_renderpass_data()
	{
		m_render_pass->update_camera(m_camera);
		m_render_pass->reserve_objects(m_current_map->num_objects());
		for (const auto& obj : MapObjectsRange(m_current_map))
		{
			m_render_pass->push_object();
		}
	}
	void ViewportWidget::draw_imgui()
	{
		ImGuiWindowFlags window_flags{};
		window_flags |= ImGuiWindowFlags_NoTitleBar;

		if (auto widget = rex::imgui::ScopedWidget("Viewport", nullptr, window_flags))
		{
			// Make sure that we hide the tab bar, it doesn't make sense to have it for our viewport
			ImGui::GetCurrentWindow()->WindowClass.DockNodeFlagsOverrideSet |= ImGuiDockNodeFlags_HiddenTabBar;

			// Use the renderpass's render target as image for the viewport
			const rex::gfx::RenderTarget* render_target = m_render_pass->render_target();
			rex::imgui::image(render_target);
		}
	}
}