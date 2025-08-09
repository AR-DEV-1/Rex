//#include "rex_engine/gfx/rendering/render_passes/tile_pass.h"
//
//#include "rex_std/vector.h"
//
//namespace rex
//{
//	namespace gfx
//	{
//		TileMap::TileMap()
//		{}
//		TileMap::TileMap(s32 width, s32 height)
//			: m_tile_indices(rsl::make_unique<s8>(width* height))
//		{}
//
//		TilePass::TilePass(const TilePassDesc& desc)
//			: rex::gfx::RenderPass(desc.renderpass_desc)
//			, m_tileset_asset(desc.tileset)
//			, m_tilemap(desc.total_tilemap_width, desc.total_tilemap_height)
//		{
//			REX_ASSERT_X(desc.renderpass_desc.framebuffer_desc.size() == 1, "You can only have 1 render target output for a tile render pass");
//			REX_ASSERT_X(desc.renderpass_desc.framebuffer_desc.front().use_swapchain() == false, "A tile render pass cannot directly render to the swapchain");
//
//			init();
//		}
//
//		// Update the camera where to render from
//		void TilePass::update_camera(rsl::pointi8 cameraPos)
//		{
//			m_camera_pos = cameraPos;
//		}
//
//		// Return the render target that's used as output for this pass
//		const rex::gfx::RenderTarget* TilePass::render_target() const
//		{
//			// Return the first render target of the base class
//			// We should only have 1
//			return rex::gfx::RenderPass::render_target(0);
//		}
//
//		// Update the tileset texture that's used to render the tiles
//		void TilePass::update_tileset(const rex::TilesetAsset* tileset)
//		{
//			m_tileset_asset = tileset;
//		}
//
//		void TilePass::pre_pass() 
//		{
//			update_tile_data(m_camera_pos);
//
//			upload_tile_data_to_gpu();
//		}
//		void TilePass::run_pass() 
//		{
//			auto render_ctx = rex::gfx::gal::instance()->new_render_ctx();
//
//			// Bind data to the render pipeline
//			bind_to(render_ctx.get());
//			render_ctx->set_vertex_buffer(m_tiles_vb_gpu.get());
//			render_ctx->set_index_buffer(m_tiles_ib_gpu.get());
//
//			// Perform the draw calls
//			const s32 index_count_per_instance = 6;
//			const s32 instance_count = num_screen_tiles();
//			render_ctx->draw_indexed_instanced(index_count_per_instance, instance_count, 0, 0, 0);
//		}
//		void TilePass::post_pass() 
//		{}
//
//		s32 TilePass::num_screen_tiles() const
//		{
//			return rendertarget_width_in_tiles() * rendertarget_height_in_tiles();
//		}
//
//#pragma region Initialization
//		void TilePass::init()
//		{
//			// The tile cache is like a map matrix but it holds indices on a tile level instead of block level
//			m_screen_tilemap_cpu = rsl::make_unique<TileMap>(rendertarget_width_in_tiles(), rendertarget_height_in_tiles());
//
//			// Initialize all the GPU resources
//			init_per_tile_vb();
//			init_ib();
//			init_cb();
//			init_tilemap_gpu();
//			init_sampler();
//			init_shader_params();
//		}
//		void TilePass::init_per_tile_vb()
//		{
//			// The goal of this func is to create a vertex buffer that holds the following data
//			//
//			// v0                          v1
//			//  +--------------------------+
//			//  |                          |
//			//  |                          |
//			//  |                          |
//			//  |                          |
//			//  |                          |
//			//  |                          |
//			//  |                          |
//			//  |                          |
//			//  |                          |
//			//  +--------------------------+
//			// v2                          v3
//
//			// We do this by calculating the screen coordinates (in NDC) of a single tile
//			// if it were drawn in the top left
//			// The shader is responsible for moving this vertex into the correct location on screen
//			// based on the instance ID of the tile being drawn
//
//			// Because NDC coordinates go from -1 to 1, divide 2.0f by the width/height
//			f32 inv_tile_screen_width = 2.0f / rendertarget_width_in_tiles();
//			f32 inv_tile_screen_height = 2.0f / rendertarget_height_in_tiles();
//
//			f32 uv_width = static_cast<f32>(tile_width_px()) / m_tileset_asset->tileset_texture()->texture_resource()->width();
//			f32 uv_height = static_cast<f32>(tile_height_px()) / m_tileset_asset->tileset_texture()->texture_resource()->height();
//
//			constexpr s32 num_vertices_per_tile = 4;
//
//			rsl::array<TileVertex, num_vertices_per_tile> tile_vertices{};
//			// A single tile starts from the top left and spans the entire screen
//			// NDC coordinates for a screen go from -1 to 1, meaning that width and height should be of a unit of 2
//			tile_vertices[0] = TileVertex{ rsl::point<f32>(0,                                           0), rsl::point<f32>(0.0f, 0.0f) };            // top left
//			tile_vertices[1] = TileVertex{ rsl::point<f32>(inv_tile_screen_width,                       0), rsl::point<f32>(uv_width, 0.0f) };        // top right
//			tile_vertices[2] = TileVertex{ rsl::point<f32>(0,                     -inv_tile_screen_height), rsl::point<f32>(0.0f, uv_height) };       // bottom left
//			tile_vertices[3] = TileVertex{ rsl::point<f32>(inv_tile_screen_width, -inv_tile_screen_height), rsl::point<f32>(uv_width, uv_height) };   // bottom right
//
//			// It's possible we're updating the tiles vertex buffer due to a map transition (as UV could be different)
//			// In which case we don't need to allocate a new one, just update the existing one
//			if (!m_tiles_vb_gpu)
//			{
//				m_tiles_vb_gpu = rex::gfx::gal::instance()->create_vertex_buffer(num_vertices_per_tile, sizeof(TileVertex));
//			}
//
//			auto render_ctx = rex::gfx::gal::instance()->new_render_ctx();
//			render_ctx->update_buffer(m_tiles_vb_gpu.get(), tile_vertices.data(), tile_vertices.size() * sizeof(TileVertex));
//			render_ctx->transition_buffer(m_tiles_vb_gpu.get(), rex::gfx::ResourceState::VertexAndConstantBuffer);
//		}
//		void TilePass::init_ib()
//		{
//			constexpr s32 num_indices_per_tile = 6;
//			rsl::unique_array<u16> tiles_ib_cpu = rsl::make_unique<u16[]>(num_indices_per_tile);
//
//			// first triangle
//			tiles_ib_cpu[0] = 0;
//			tiles_ib_cpu[1] = 1;
//			tiles_ib_cpu[2] = 2;
//
//			// second triangle
//			tiles_ib_cpu[3] = 1;
//			tiles_ib_cpu[4] = 3;
//			tiles_ib_cpu[5] = 2;
//
//			m_tiles_ib_gpu = rex::gfx::gal::instance()->create_index_buffer(num_indices_per_tile, rex::gfx::IndexBufferFormat::Uint16);
//
//			auto render_ctx = rex::gfx::gal::instance()->new_render_ctx();
//			render_ctx->update_buffer(m_tiles_ib_gpu.get(), tiles_ib_cpu.get(), tiles_ib_cpu.byte_size());
//			render_ctx->transition_buffer(m_tiles_ib_gpu.get(), rex::gfx::ResourceState::IndexBuffer);
//		}
//		void TilePass::init_cb()
//		{
//			TileRenderingMetaData tile_rendering_meta_data{};
//			tile_rendering_meta_data.texture_tiles_per_row = texture_width_in_tiles();
//			tile_rendering_meta_data.inv_texture_width = 1.0f / m_tileset_asset->tileset_texture()->texture_resource()->width();
//			tile_rendering_meta_data.inv_texture_height = 1.0f / m_tileset_asset->tileset_texture()->texture_resource()->height();
//
//			tile_rendering_meta_data.screen_width_in_tiles = rendertarget_width_in_tiles();
//			tile_rendering_meta_data.inv_tile_screen_width = 2.0f / render_target()->width();
//			tile_rendering_meta_data.inv_tile_screen_height = 2.0f / render_target()->height();
//
//			// It's possible we're updating and not creating due to a map transition
//			// In which case we don't need to allocate a new one, just update the existing one
//			if (!m_tile_rendering_meta_data_cb)
//			{
//				m_tile_rendering_meta_data_cb = rex::gfx::gal::instance()->create_constant_buffer(sizeof(tile_rendering_meta_data));
//			}
//
//			auto render_ctx = rex::gfx::gal::instance()->new_render_ctx();
//			render_ctx->update_buffer(m_tile_rendering_meta_data_cb.get(), &tile_rendering_meta_data, sizeof(tile_rendering_meta_data));
//		}
//		void TilePass::init_tilemap_gpu()
//		{
//			// We don't have any data for this buffer yet, so we don't upload anything
//			m_screen_tilemap_gpu = rex::gfx::gal::instance()->create_unordered_access_buffer(num_screen_tiles());
//			auto render_context = rex::gfx::gal::instance()->new_render_ctx();
//			render_context->transition_buffer(m_screen_tilemap_gpu.get(), rex::gfx::ResourceState::NonPixelShaderResource);
//		}
//		void TilePass::init_sampler()
//		{
//			m_default_sampler = rex::gfx::gal::instance()->common_sampler(rex::gfx::CommonSampler::Default2D);
//		}
//		void TilePass::init_shader_params()
//		{
//			set("RenderingMetaData", m_tile_rendering_meta_data_cb.get());
//			set("tile_texture", m_tileset_asset->tileset_texture()->texture_resource());
//			set("tile_instance_to_tile_idx_in_texture", m_screen_tilemap_gpu.get());
//
//			// This data should be initialized through data, not hardcoded
//			set("default_sampler", m_default_sampler);
//		}
//#pragma endregion
//
//		void TilePass::update_tile_data(const rsl::pointi8 cameraPos)
//		{
//			// Calculate the first block from which we should start drawing
//			rsl::pointi8 top_left = centre_pos_to_top_left(m_camera_pos);
//
//			// Start the loop from this block, going left to right, top to down
//			// Restricting to only the tiles that'll be rendered
//			s32 num_tile_columns = rendertarget_width_in_tiles();
//			s32 num_tile_rows = rendertarget_height_in_tiles();
//
//			// Allocate a buffer representing tile indices of a single row
//			rsl::vector<s8> tilemap_row;
//			tilemap_row.reserve(num_tile_columns);
//
//			for (s32 y = 0; y < num_tile_rows; ++y)
//			{
//				tilemap_row.clear();
//
//				for (s32 x = 0; x < num_tile_columns; ++x)
//				{
//					// Get the tile coord of the tile we're currently processing
//					rsl::pointi8 coord = top_left;
//					coord.x += static_cast<s8>(x);
//					coord.y += static_cast<s8>(y);
//
//					s8 tile_idx = m_tileset_asset->tile_idx_at(coord);
//					tilemap_row.push_back(tile_idx);
//
//					// Get the block the tile belongs to
//					s32 block_idx = m_tilemap.index_at_blockcoord(coord);
//					const rex::Block& block = m_blockset->block(block_idx);
//
//					// Get the tile coordinate of the the first tile in the block (which is top left)
//					rsl::pointi8 block_top_left = block_top_left_coord(coord);
//
//					// Get the relative vector from this first tile to the current tile
//					// Based on that, calculate which tile in the block we're currently processing
//					rsl::pointi8 coord_rel_to_block = coord - block_top_left;
//					u8 tile_idx = block.index_at(coord_rel_to_block);
//
//					// Store the tile index in the cache
//					s32 tile_instance_idx = y * num_tile_columns + x;
//					m_tile_instance_to_tile_idx_cpu[tile_instance_idx] = tile_idx;
//				}
//
//				m_screen_tilemap_cpu->update_row(y, tilemap_row.data(), tilemap_row.size());
//			}
//		}
//		void TilePass::upload_tile_data_to_gpu()
//		{
//			auto render_ctx = rex::gfx::gal::instance()->new_render_ctx();
//			render_ctx->update_buffer(m_screen_tilemap_gpu.get(), m_screen_tilemap_cpu->tile_indices(), m_screen_tilemap_cpu->count());
//			render_ctx->transition_buffer(m_screen_tilemap_gpu.get(), rex::gfx::ResourceState::NonPixelShaderResource);
//		}
//
//		rsl::pointi8 TilePass::centre_pos_to_top_left(rsl::pointi8 coord)
//		{
//			// The player is always in tile coord {8, 8};
//
//			rsl::pointi8 screen_top_left = coord;
//			screen_top_left.x -= constants::player_render_position_top_left.x;
//			screen_top_left.y -= constants::player_render_position_top_left.y;
//
//			return screen_top_left;
//		}
//
//		rsl::pointi8 TilePass::block_to_tile_coord(rsl::pointi8 coord)
//		{
//			rsl::pointi8 tile_coord{};
//			tile_coord.x = coord.x * constants::g_num_tiles_per_block_row;
//			tile_coord.y = coord.y * constants::g_num_tiles_per_block_column;
//
//			return tile_coord;
//		}
//		rsl::pointi8 TilePass::tile_to_block_coord(rsl::pointi8 coord)
//		{
//			rsl::pointi8 block_coord{};
//			block_coord.x = coord.x / constants::g_num_tiles_per_block_row;
//			block_coord.y = coord.y / constants::g_num_tiles_per_block_column;
//
//			return block_coord;
//		}
//		rsl::pointi8 TilePass::block_top_left_coord(rsl::pointi8 coord)
//		{
//			rsl::pointi8 block_coord = tile_to_block_coord(coord);
//			return block_to_tile_coord(block_coord);
//		}
//
//		f32 TilePass::rendertarget_width_in_tiles() const
//		{
//			return render_target()->width() / static_cast<f32>(tile_width_px());
//		}
//		f32 TilePass::rendertarget_height_in_tiles() const
//		{
//			return render_target()->height() / static_cast<f32>(tile_height_px());
//		}
//		f32 TilePass::texture_width_in_tiles() const
//		{
//			return m_tileset_asset->tileset_texture()->texture_resource()->width() / static_cast<f32>(m_tileset_asset->tile_width_px());
//		}
//		f32 TilePass::texture_height_in_tiles() const
//		{
//			return m_tileset_asset->tileset_texture()->texture_resource()->height() / static_cast<f32>(m_tileset_asset->tile_height_px());
//		}
//		s32 TilePass::tile_width_px() const
//		{
//			return m_tileset_asset->tile_width_px();
//		}
//		s32 TilePass::tile_height_px() const
//		{
//			return m_tileset_asset->tile_height_px();
//		}
//
//	}
//}