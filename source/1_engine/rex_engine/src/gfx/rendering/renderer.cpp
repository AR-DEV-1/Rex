#include "rex_engine/gfx/rendering/renderer.h"

#include "rex_engine/gfx/graphics.h"

namespace rex
{
	namespace gfx
	{
		void Renderer::render_tilemap(const TilemapRenderRequest& tilemapRenderRequest)
		{
			TilemapRenderData tilemap_render_data{};

			// create the vertex buffer
			// -----------------------------------------
			s32 render_target_width = 0;
			s32 render_target_height = 0;

			s32 tile_width = 8;
			s32 tile_height = 8;

			s32 render_target_width_in_tiles = render_target_width * tile_width;
			s32 render_target_height_in_tiles = render_target_height * tile_height;

			f32 inv_tile_width = 2.0f / render_target_width_in_tiles;
			f32 inv_tile_height = 2.0f / render_target_height_in_tiles;

			f32 uv_width = tile_width / tilemapRenderRequest.tileset->width();
			f32 uv_height = tile_height / tilemapRenderRequest.tileset->height();

			const s32 num_vertices_per_tile = 4;

			rsl::array<TileVertex, num_vertices_per_tile> tile_vertices{};

			tile_vertices[0] = TileVertex{ rsl::point<f32>(0,              0),                rsl::point<f32>(0.0f,     0.0f) };
			tile_vertices[1] = TileVertex{ rsl::point<f32>(inv_tile_width, 0),                rsl::point<f32>(uv_width, 0.0f) };
			tile_vertices[2] = TileVertex{ rsl::point<f32>(0,              -inv_tile_height), rsl::point<f32>(0.0f,     uv_height) };
			tile_vertices[3] = TileVertex{ rsl::point<f32>(inv_tile_width, -inv_tile_height), rsl::point<f32>(uv_width, uv_height) };

			tilemap_render_data.tiles_vb_gpu = rex::gfx::gal::instance()->create_vertex_buffer(num_vertices_per_tile, sizeof(TileVertex));

			auto render_ctx = gal::instance()->new_render_ctx();
			render_ctx->update_buffer(tilemap_render_data.tiles_vb_gpu.get(), tile_vertices.data(), tile_vertices.size() * sizeof(TileVertex));
			render_ctx->transition_buffer(tilemap_render_data.tiles_vb_gpu.get(), ResourceState::VertexAndConstantBuffer);


			// create the index buffer
			// -----------------------------------------
			const s32 num_indices_per_tile = 6;
			rsl::array<u16, 6> tile_ib{};

			tile_ib[0] = 0;
			tile_ib[1] = 1;
			tile_ib[0] = 2;

			tile_ib[3] = 1;
			tile_ib[4] = 3;
			tile_ib[5] = 2;

			tilemap_render_data.tiles_ib_gpu = gal::instance()->create_index_buffer(num_indices_per_tile, IndexBufferFormat::Uint16);

			// create the constant buffer
			// -----------------------------------------
			render_ctx->update_buffer(tilemap_render_data.tiles_ib_gpu.get(), tile_ib.data(), tile_ib.size() * sizeof(tile_ib[0]));
			render_ctx->transition_buffer(tilemap_render_data.tiles_ib_gpu.get(), ResourceState::IndexBuffer);

			// create the tile indices buffer
			// -----------------------------------------
			TilemapRenderingMetaData render_metadata{};
			render_metadata.texture_tiles_per_row = tilemapRenderRequest.tileset->width_in_tiles();
			render_metadata.inv_texture_width = 1.0f / tilemapRenderRequest.tileset->width();
			render_metadata.inv_texture_height = 1.0f / tilemapRenderRequest.tileset->height();

			render_metadata.screen_width_in_tiles = tilemapRenderRequest.tilemap->width_in_tiles();
			render_metadata.inv_screen_width = inv_tile_width;
			render_metadata.inv_screen_height = inv_tile_height;


			tilemap_render_data.tile_render_info = gal::instance()->create_constant_buffer(sizeof(TilemapRenderingMetaData));

			render_ctx->update_buffer(tilemap_render_data.tile_render_info.get(), &render_metadata, sizeof(render_metadata));
		}

		namespace renderer
		{
			globals::GlobalUniquePtr<Renderer> g_renderer;
			void init(globals::GlobalUniquePtr<Renderer> renderer)
			{
				g_renderer = rsl::move(g_renderer);
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