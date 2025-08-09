//#pragma once
//
//#include "rex_engine/gfx/rendering/render_pass.h"
//#include "rex_engine/assets/tileset_asset.h"
//
//namespace rex
//{
//	namespace gfx
//	{
//		// A tilemap has 3 different kind of coordinate matrices
//		// tile coordinate: lower possible coordinate value (1x1 matrix)
//		// square coordinate: a square is a matrix of 2x2 tiles
//		// block coordinate: a block is a matrix of 4x4 tiles
//		// Each coordinate is always used to represent the top left tile of the matrix
//		// so a coordinate of (1,1) would represent the following
//		//
//		// TILES
//		//
//		// +---+---+---+---+---+---+---+---+---+---+---+---+
//		// |   |   |   |   |   |   |   |   |   |   |   |   |
//		// +---+---+---+---+---+---+---+---+---+---+---+---+
//		// |   | x |   |   |   |   |   |   |   |   |   |   |
//		// +---+---+---+---+---+---+---+---+---+---+---+---+
//		// |   |   |   |   |   |   |   |   |   |   |   |   |
//		// +---+---+---+---+---+---+---+---+---+---+---+---+
//		// |   |   |   |   |   |   |   |   |   |   |   |   |
//		// +---+---+---+---+---+---+---+---+---+---+---+---+
//		// |   |   |   |   |   |   |   |   |   |   |   |   |
//		// +---+---+---+---+---+---+---+---+---+---+---+---+
//		// |   |   |   |   |   |   |   |   |   |   |   |   |
//		// +---+---+---+---+---+---+---+---+---+---+---+---+
//		// |   |   |   |   |   |   |   |   |   |   |   |   |
//		// +---+---+---+---+---+---+---+---+---+---+---+---+
//		// |   |   |   |   |   |   |   |   |   |   |   |   |
//		// +---+---+---+---+---+---+---+---+---+---+---+---+
//		//
//		// SQUARES
//		//
//		// +---+---+---+---+---+---+---+---+---+---+---+---+
//		// |   |   |   |   |   |   |   |   |   |   |   |   |
//		// +---+---+---+---+---+---+---+---+---+---+---+---+
//		// |   |   |   |   |   |   |   |   |   |   |   |   |
//		// +---+---+---+---+---+---+---+---+---+---+---+---+
//		// |   |   | x | x |   |   |   |   |   |   |   |   |
//		// +---+---+---+---+---+---+---+---+---+---+---+---+
//		// |   |   | x | x |   |   |   |   |   |   |   |   |
//		// +---+---+---+---+---+---+---+---+---+---+---+---+
//		// |   |   |   |   |   |   |   |   |   |   |   |   |
//		// +---+---+---+---+---+---+---+---+---+---+---+---+
//		// |   |   |   |   |   |   |   |   |   |   |   |   |
//		// +---+---+---+---+---+---+---+---+---+---+---+---+
//		// |   |   |   |   |   |   |   |   |   |   |   |   |
//		// +---+---+---+---+---+---+---+---+---+---+---+---+
//		// |   |   |   |   |   |   |   |   |   |   |   |   |
//		// +---+---+---+---+---+---+---+---+---+---+---+---+
//		//
//		//
//		// BLOCKS
//		//
//		// +---+---+---+---+---+---+---+---+---+---+---+---+
//		// |   |   |   |   |   |   |   |   |   |   |   |   |
//		// +---+---+---+---+---+---+---+---+---+---+---+---+
//		// |   |   |   |   |   |   |   |   |   |   |   |   |
//		// +---+---+---+---+---+---+---+---+---+---+---+---+
//		// |   |   |   |   |   |   |   |   |   |   |   |   |
//		// +---+---+---+---+---+---+---+---+---+---+---+---+
//		// |   |   |   |   |   |   |   |   |   |   |   |   |
//		// +---+---+---+---+---+---+---+---+---+---+---+---+
//		// |   |   |   |   | x | x | x | x |   |   |   |   |
//		// +---+---+---+---+---+---+---+---+---+---+---+---+
//		// |   |   |   |   | x | x | x | x |   |   |   |   |
//		// +---+---+---+---+---+---+---+---+---+---+---+---+
//		// |   |   |   |   | x | x | x | x |   |   |   |   |
//		// +---+---+---+---+---+---+---+---+---+---+---+---+
//		// |   |   |   |   | x | x | x | x |   |   |   |   |
//		// +---+---+---+---+---+---+---+---+---+---+---+---+
//
//		class TileMap
//		{
//		public:
//			TileMap();
//			TileMap(s32 width, s32 height);
//
//			void update_row(s32 rowIdx, s8* data, s32 count);
//			void update_column(s32 columnIdx);
//
//			const s8* tile_indices() const;
//			s32 count() const;
//
//		private:
//			rsl::unique_ptr<s8> m_tile_indices;
//		};
//
//		struct TileVertex
//		{
//			rsl::point<f32> pos;
//			rsl::point<f32> uv;
//		};
//
//		struct TileRenderingMetaData
//		{
//			// Tile texture data
//			u32 texture_tiles_per_row;   // the number of tiles per row in the tileset texture
//			f32 inv_texture_width;       // the inverse width of the tileset texture, in pixels
//			f32 inv_texture_height;      // the inverse height of the tileset texture, in pixels
//
//			// Render target data
//			u32 screen_width_in_tiles;   // the number of tiles we render on a single row
//			f32 inv_tile_screen_width;   // the inverse of the width of a single tile on the screen
//			f32 inv_tile_screen_height;  // the inverse of the height of a single tile on the screen
//		};
//
//		struct TilePassDesc
//		{
//			RenderPassDesc renderpass_desc;
//			const rex::TilesetAsset* tileset;
//
//			// Thse are the dimensions for the tilemap that we don't immediatelly render
//			// This tilemap gets referenced and we always days a subregion of this tilemap to the screen.
//			TileMap backend_tilemap;
//		};
//
//		// This represents the renderpass that's used to render tiles
//		// A 2D game often uses tiles instead of meshes
//		// The tilepass only renders the tiles that are in view.
//		// It starts in the top left and assigns tile indices row by row
//		// The resulting matrix is send to the GPU, which uses these indices
//		// to draw the tile at the correct location
//		class TilePass : public rex::gfx::RenderPass
//		{
//		public:
//			TilePass(const TilePassDesc& desc);
//
//			// Updates the tilemap that needs to be rendered to screen
//			// The position given is the position from the top left of the tilemap
//			// If you're tilemap is 100x100, a position of (20,20), would render (20, 20) at the middle of your screen
//			// and using the settings provided to calculate the other tiles that need to be rendered
//			// If anything falls out of bounds of the tilemap, nothing will be rendered there
//			void update_tile_data(const rsl::pointi8 cameraPos);
//
//			// Update the camera where to render from
//			void update_camera(rsl::pointi8 cameraPos);
//			// Return the render target that's used as output for this pass
//			const rex::gfx::RenderTarget* render_target() const;
//
//			// Update the tileset texture that's used to render the tiles
//			void update_tileset(const rex::TilesetAsset* tileset);
//
//			void pre_pass() override;
//			void run_pass() override;
//			void post_pass() override;
//
//			s32 tile_width_px() const;
//			s32 tile_height_px() const;
//
//			s32 width_in_tiles() const;
//			s32 width_in_squares() const;
//			s32 width_in_blocks() const;
//
//			s32 height_in_tiles() const;
//			s32 height_in_squares() const;
//			s32 height_in_blocks() const;
//
//
//
//
//
//		private:
//			s32 num_screen_tiles() const;
//
//			void init();
//			void init_per_tile_vb();
//			void init_ib();
//			void init_cb();
//			void init_tilemap_gpu();
//			void init_sampler();
//			void init_shader_params();
//
//			void upload_tile_data_to_gpu();
//
//			f32 rendertarget_width_in_tiles() const;
//			f32 rendertarget_height_in_tiles() const;
//			f32 texture_width_in_tiles() const;
//			f32 texture_height_in_tiles() const;
//
//			rsl::pointi8 centre_pos_to_top_left(rsl::pointi8 coord);
//
//			rsl::pointi8 block_to_tile_coord(rsl::pointi8 coord);
//			rsl::pointi8 tile_to_block_coord(rsl::pointi8 coord);
//			rsl::pointi8 block_top_left_coord(rsl::pointi8 coord);
//
//		private:
//			const TilesetAsset* m_tileset_asset;
//			rsl::unique_ptr<TileMap> m_screen_tilemap_cpu;
//
//			//s32 m_tile_width_px;   // the width of a single tile in pixels
//			//s32 m_tile_height_px;  // the height of a single tile in pixels
//
//			rsl::pointi8 m_camera_pos;
//			//const rex::Blockset* m_blockset;   // The blockset to use to retrieve the tileset indices from
//			MapMatrix m_map_matrix;
//
//			// Gpu resources required for rendering tiles
//			rsl::unique_ptr<rex::gfx::VertexBuffer>   m_tiles_vb_gpu;																// This holds the 4 vertices used to make up a plane
//			rsl::unique_ptr<rex::gfx::IndexBuffer>    m_tiles_ib_gpu;																// This holds the indices for every tile. This never changes at runtime
//			rsl::unique_ptr<rex::gfx::ConstantBuffer> m_tile_rendering_meta_data_cb;								// This holds the extra metadata the shader needs to calculate the UVs for each instance
//			rsl::unique_ptr<rex::gfx::UnorderedAccessBuffer> m_screen_tilemap_gpu;									// This holds the mapping of each tile instance to the tile idx in the texture
//
//			//const rex::gfx::Texture2D* m_tileset_texture;				// the tileset texture to use
//			const rex::gfx::Sampler2D* m_default_sampler;				// The sampler for the tileset texture
//			TileMap m_tilemap;
//
//		};
//	}
//}