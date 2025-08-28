#include "rex_engine/gfx/rendering/renderer.h"

#include "rex_engine/gfx/graphics.h"
#include "rex_engine/gfx/rendering/render_pass.h"
#include "rex_engine/gfx/system/shader_library.h"

#include "rex_engine/filesystem/path.h"

#include "rex_std/bonus/math.h"

namespace rex
{
	namespace gfx
	{
		struct TileVertex
		{
			rsl::point<f32> pos;
			rsl::point<f32> uv;
		};

		Renderer::Renderer()
		{
			init_ib();
		}

		void Renderer::render_tilemap(const TilemapRenderRequest& tilemapRenderRequest)
		{
			TilemapRenderData tilemap_render_data{};

			// create the vertex buffer
			// -----------------------------------------

			// Perform the calculations required to initialize the VB and other render data
			s32 render_target_width = tilemapRenderRequest.render_target->width();
			s32 render_target_height = tilemapRenderRequest.render_target->height();
			rsl::pointi8 tile_size = tilemapRenderRequest.tileset->tile_size();

			s32 render_target_width_in_tiles = render_target_width / tile_size.x;
			s32 render_target_height_in_tiles = render_target_height / tile_size.y;

			f32 inv_tile_width = 2.0f / render_target_width_in_tiles;
			f32 inv_tile_height = 2.0f / render_target_height_in_tiles;

			s32 tileset_width = tilemapRenderRequest.tileset->tileset_texture()->texture_resource()->width();
			s32 tileset_height = tilemapRenderRequest.tileset->tileset_texture()->texture_resource()->height();

			f32 uv_width = tile_size.x / (f32)tileset_width;
			f32 uv_height = tile_size.y / (f32)tileset_height;

			const s32 num_vertices_per_tile = 4;

			rsl::array<TileVertex, num_vertices_per_tile> tile_vertices{};

			tile_vertices[0] = TileVertex{ rsl::point<f32>(0,              0),                rsl::point<f32>(0.0f,     0.0f) };
			tile_vertices[1] = TileVertex{ rsl::point<f32>(inv_tile_width, 0),                rsl::point<f32>(uv_width, 0.0f) };
			tile_vertices[2] = TileVertex{ rsl::point<f32>(0,              -inv_tile_height), rsl::point<f32>(0.0f,     uv_height) };
			tile_vertices[3] = TileVertex{ rsl::point<f32>(inv_tile_width, -inv_tile_height), rsl::point<f32>(uv_width, uv_height) };

			tilemap_render_data.tiles_vb_gpu = rex::gfx::gal::instance()->create_vertex_buffer(num_vertices_per_tile, sizeof(TileVertex));
			tilemap_render_data.tiles_ib_gpu = m_index_buffer.get();

			// create the constant buffer
			// -----------------------------------------
			struct TilemapRenderingMetaData
			{
				// Tile texture data
				u32 texture_tiles_per_row;   // the number of tiles per row in the tileset texture
				f32 inv_texture_width;       // the inverse width of the tileset texture, in pixels
				f32 inv_texture_height;      // the inverse height of the tileset texture, in pixels

				// Render target data
				u32 screen_width_in_tiles;   // the number of tiles we render on a single row
				f32 inv_tile_screen_width;   // the inverse of the width of a single tile on the screen
				f32 inv_tile_screen_height;  // the inverse of the height of a single tile on the screen
			};

			TilemapRenderingMetaData render_metadata{};
			render_metadata.texture_tiles_per_row = tileset_width / tile_size.x;
			render_metadata.inv_texture_width = uv_width;
			render_metadata.inv_texture_height = uv_height;

			render_metadata.screen_width_in_tiles = tilemapRenderRequest.tilemap->width_in_tiles();
			render_metadata.inv_tile_screen_width = inv_tile_width;
			render_metadata.inv_tile_screen_height = inv_tile_height;

			tilemap_render_data.tile_render_info = gal::instance()->create_constant_buffer(sizeof(TilemapRenderingMetaData));

			// create the tile indices buffer
			// -----------------------------------------
			tilemap_render_data.tile_indices_buffer = gal::instance()->create_unordered_access_buffer(tilemapRenderRequest.tilemap->num_tiles());
			const s32 tile_byte_size = sizeof(tilemapRenderRequest.tilemap->tiles()[0]);
			rex::gfx::Sampler2D* default_sampler = rex::gfx::gal::instance()->common_sampler(rex::gfx::CommonSampler::Default2D);

			rex::gfx::RenderPassDesc render_pass_desc{};

			render_pass_desc.name = "Tile Renderer";

			render_pass_desc.pso_desc.output_merger.raster_state = rex::gfx::gal::instance()->common_raster_state(rex::gfx::CommonRasterState::DefaultDepth);

			// We're rendering directly to the back buffer
			render_pass_desc.framebuffer_desc.emplace_back(rex::gfx::swapchain_frame_buffer_handle());

			// Assign the shaders used for the tile renderer
			render_pass_desc.pso_desc.shader_pipeline.vs = rex::gfx::shader_lib::instance()->load(rex::path::join(rex::engine::instance()->project_root(), "shaders", "render_tile_vertex.hlsl"), rex::gfx::ShaderType::Vertex);
			render_pass_desc.pso_desc.shader_pipeline.ps = rex::gfx::shader_lib::instance()->load(rex::path::join(rex::engine::instance()->project_root(), "shaders", "render_tile_pixel.hlsl"), rex::gfx::ShaderType::Pixel);

			render_pass_desc.pso_desc.input_layout =
			{
				// Per vertex data
				rex::gfx::InputLayoutElementDesc{ rex::gfx::ShaderSemantic::Position, rex::gfx::ShaderArithmeticType::Float2 },
				rex::gfx::InputLayoutElementDesc{ rex::gfx::ShaderSemantic::TexCoord, rex::gfx::ShaderArithmeticType::Float2 },
			};

			rsl::unique_ptr<rex::gfx::RenderPass> render_pass = rsl::make_unique<rex::gfx::RenderPass>(render_pass_desc);
			render_pass->set("tile_texture", tilemapRenderRequest.tileset->tileset_texture()->texture_resource());
			render_pass->set("default_sampler", default_sampler);
			render_pass->set("RenderingMetaData", tilemap_render_data.tile_render_info.get());
			render_pass->set("TileIndexIntoTextureBuffer", tilemap_render_data.tile_indices_buffer.get());

			auto render_ctx = gal::instance()->new_render_ctx();
			render_ctx->update_buffer(tilemap_render_data.tiles_vb_gpu.get(), tile_vertices.data(), tile_vertices.size() * sizeof(TileVertex));
			render_ctx->transition_buffer(tilemap_render_data.tiles_vb_gpu.get(), ResourceState::VertexAndConstantBuffer);
			render_ctx->update_buffer(tilemap_render_data.tile_render_info.get(), &render_metadata, sizeof(render_metadata));
			render_ctx->update_buffer(tilemap_render_data.tile_indices_buffer.get(), tilemapRenderRequest.tilemap->tiles(), tilemapRenderRequest.tilemap->num_tiles() * tile_byte_size);
			render_ctx->transition_buffer(tilemap_render_data.tile_indices_buffer.get(), ResourceState::NonPixelShaderResource);

			render_pass->bind_to(render_ctx.get());

			// Bind all the resources to the gfx pipeline
			render_ctx->set_vertex_buffer(tilemap_render_data.tiles_vb_gpu.get(), 0);
			render_ctx->set_index_buffer(tilemap_render_data.tiles_ib_gpu);
			render_ctx->set_render_target(tilemapRenderRequest.render_target);

			f32 viewport_width = static_cast<f32>(render_target_width);
			f32 viewport_height = static_cast<f32>(render_target_height);
			Viewport viewport = { glm::vec2(0.0f, 0.0f), viewport_width, viewport_height, 0.0f, 1.0f };
			render_ctx->set_viewport(viewport);

			ScissorRect rect = { 0, 0, viewport_width/2, viewport_height };
			render_ctx->set_scissor_rect(rect);

			//render_ctx->use_swapchain_framebuffer();

			// Send the draw command
			const s32 index_count_per_instance = 6;
			const s32 instance_count = tilemapRenderRequest.tilemap->num_tiles();
			render_ctx->draw_indexed_instanced(index_count_per_instance, instance_count, 0, 0, 0);

			render_ctx->execute_on_gpu(WaitForFinish::yes);
		}

		void Renderer::init_ib()
		{
			const s32 num_indices_per_tile = 6;
			rsl::array<u16, 6> tile_ib{};

			tile_ib[0] = 0;
			tile_ib[1] = 1;
			tile_ib[2] = 2;

			tile_ib[3] = 1;
			tile_ib[4] = 3;
			tile_ib[5] = 2;

			m_index_buffer = gal::instance()->create_index_buffer(num_indices_per_tile, IndexBufferFormat::Uint16);

			// create the constant buffer
			// -----------------------------------------
			auto render_ctx = gal::instance()->new_render_ctx();
			render_ctx->update_buffer(m_index_buffer.get(), tile_ib.data(), tile_ib.size() * sizeof(tile_ib[0]));
			render_ctx->transition_buffer(m_index_buffer.get(), ResourceState::IndexBuffer);
		}

		namespace renderer
		{
			globals::GlobalUniquePtr<Renderer> g_renderer;
			void init(globals::GlobalUniquePtr<Renderer> renderer)
			{
				g_renderer = rsl::move(renderer);
			}
			Renderer* instance()
			{
				return g_renderer.get();
			}
			void shutdown()
			{
				g_renderer.reset();
			}
		}
	}
}