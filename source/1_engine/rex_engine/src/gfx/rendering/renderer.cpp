#include "rex_engine/gfx/rendering/renderer.h"

#include "rex_engine/gfx/graphics.h"

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

			s32 render_target_width_in_tiles = render_target_width * tile_size.x;
			s32 render_target_height_in_tiles = render_target_height * tile_size.y;

			f32 inv_tile_width = 2.0f / render_target_width_in_tiles;
			f32 inv_tile_height = 2.0f / render_target_height_in_tiles;

			f32 uv_width = tile_size.x / tilemapRenderRequest.tileset->width();
			f32 uv_height = tile_size.y / tilemapRenderRequest.tileset->height();

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
			render_metadata.texture_tiles_per_row = tilemapRenderRequest.tileset->width_in_tiles();
			render_metadata.inv_texture_width = 1.0f / tilemapRenderRequest.tileset->width();
			render_metadata.inv_texture_height = 1.0f / tilemapRenderRequest.tileset->height();

			render_metadata.screen_width_in_tiles = tilemapRenderRequest.tilemap->width_in_tiles();
			render_metadata.inv_tile_screen_width = inv_tile_width;
			render_metadata.inv_tile_screen_height = inv_tile_height;


			tilemap_render_data.tile_render_info = gal::instance()->create_constant_buffer(sizeof(TilemapRenderingMetaData));

			auto render_ctx = gal::instance()->new_render_ctx();
			render_ctx->update_buffer(tilemap_render_data.tiles_vb_gpu.get(), tile_vertices.data(), tile_vertices.size() * sizeof(TileVertex));
			render_ctx->transition_buffer(tilemap_render_data.tiles_vb_gpu.get(), ResourceState::VertexAndConstantBuffer);
			render_ctx->update_buffer(tilemap_render_data.tile_render_info.get(), &render_metadata, sizeof(render_metadata));
		}

		void Renderer::init_ib()
		{
			const s32 num_indices_per_tile = 6;
			rsl::array<u16, 6> tile_ib{};

			tile_ib[0] = 0;
			tile_ib[1] = 1;
			tile_ib[0] = 2;

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