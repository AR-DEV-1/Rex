#pragma once

#include "rex_engine/gfx/rendering/render_pass.h"
#include "rex_engine/assets/tileset_asset.h"

namespace rex
{
	namespace gfx
	{
		struct TilePassDesc
		{
			RenderPassDesc renderpass_desc;
			const rex::TilesetAsset* tileset;
		};

		// This represents the renderpass that's used to render tiles
		// A 2D game often uses tiles instead of meshes
		// The tilepass only renders the tiles that are in view.
		// It starts in the top left and assigns tile indices row by row
		// The resulting matrix is send to the GPU, which uses these indices
		// to draw the tile at the correct location
		class TilePass : public rex::gfx::RenderPass
		{
		public:
			TilePass(const TilePassDesc& desc);

			// Update the camera where to render from
			void update_camera(rsl::pointi8 cameraPos);
			// Reserve an uninitialized list of tiles to render
			void reserve_objects();
			// Push a new tile to render
			void push_object();
			// Return the render target that's used as output for this pass
			const rex::gfx::RenderTarget* render_target() const;

			// Update the tileset texture that's used to render the tiles
			void update_tileset_texture(const rex::gfx::Texture2D* texture);
			void update_dimension(s32 width, s32 height);

			void pre_pass() override;
			void run_pass() override;
			void post_pass() override;

		private:
			s32 num_tiles() const;

			void init(const TileRenderPassDesc& desc);
			void init_per_tile_vb();
			void init_ib();
			void init_cb();
			void init_tile_instance_to_tile_idx_buffer();
			void init_sampler();
			void init_shader_params();
			void init_map_matrix(const TileRenderPassDesc& desc);

			void update_tile_data(const rsl::pointi8 cameraPos);
			void upload_tile_data_to_gpu();

			f32 rendertarget_width_in_tiles() const;
			f32 rendertarget_height_in_tiles() const;
			f32 texture_width_in_tiles() const;
			f32 texture_height_in_tiles() const;

			rsl::pointi8 player_pos_to_screen_top_left(rsl::pointi8 coord);

			rsl::pointi8 block_to_tile_coord(rsl::pointi8 coord);
			rsl::pointi8 tile_to_block_coord(rsl::pointi8 coord);
			rsl::pointi8 block_top_left_coord(rsl::pointi8 coord);

		private:
			const TilesetAsset* m_tileset_asset;
			//s32 m_tile_width_px;   // the width of a single tile in pixels
			//s32 m_tile_height_px;  // the height of a single tile in pixels

			rsl::pointi8 m_camera_pos;
			//const rex::Blockset* m_blockset;   // The blockset to use to retrieve the tileset indices from
			MapMatrix m_map_matrix;

			// Gpu resources required for rendering tiles
			rsl::unique_ptr<rex::gfx::VertexBuffer>   m_tiles_vb_gpu;																// This holds the 4 vertices used to make up a plane
			rsl::unique_ptr<rex::gfx::IndexBuffer>    m_tiles_ib_gpu;																// This holds the indices for every tile. This never changes at runtime
			rsl::unique_ptr<rex::gfx::ConstantBuffer> m_tile_rendering_meta_data_cb;								// This holds the extra metadata the shader needs to calculate the UVs for each instance
			rsl::unique_ptr<rex::gfx::UnorderedAccessBuffer> m_tile_instance_to_tile_idx_gpu;				// This holds the mapping of each tile instance to the tile idx in the texture

			//const rex::gfx::Texture2D* m_tileset_texture;				// the tileset texture to use
			const rex::gfx::Sampler2D* m_default_sampler;				// The sampler for the tileset texture

			rsl::unique_array<s8> m_tile_instance_to_tile_idx_cpu;
		};
	}
}