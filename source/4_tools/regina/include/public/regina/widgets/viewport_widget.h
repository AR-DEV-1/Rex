#pragma once

#include "regina/widgets/widget.h"

#include "rex_engine/assets/texture_asset.h"

#include "rex_engine/gfx/rendering/camera.h"
#include "rex_engine/gfx/resources/render_target.h"

namespace rex
{
	class Map;
}

namespace regina
{
	class TilePass;

	// The map matrix is a matrix of the map, including the padding blocks
	// Each cell in the matrix holds a block index.
	// All blocks in the map matrix are expected to use the same blockset
	// 
	// each block is 4x4 matrix of tiles
	//  
	// +---+---+---+---+
	// | 0 | 1 | 1 | 0 |
	// +---+---+---+---+
	// | 0 | 1 | 1 | 0 |
	// +---+---+---+---+
	// | 0 | 1 | 1 | 0 |
	// +---+---+---+---+
	// | 0 | 1 | 1 | 0 |
	// +---+---+---+---+
	//
	// For a detailed explanation how maps were stored, visit: https://www.peterhajas.com/blog/pokemon_rb_map_parsing/
	//
	// 
	// A possible example of a map matrix can look like this
	//
	// # == padding block
	// c == connection block
	// m == map block
	//
	//  padding tiles                   possible connection tiles                 padding tiles
	//       |																    |															     |
	//   +---+---+														+---+---+													 +---+---+
	//   |       |														|       |													 |       |
	//   v       v														v       v													 v       v
	//                               
	// +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
	// |###|###|###|###|###|###|###|###|###| c | c | c |###|###|###|###|###|###|###|###|###|   <--+
	// +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+      |
	// |###|###|###|###|###|###|###|###|###| c | c | c |###|###|###|###|###|###|###|###|###|      +---- padding tiles
	// +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+      |
	// |###|###|###|###|###|###|###|###|###| c | c | c |###|###|###|###|###|###|###|###|###|   <--+
	// +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
	// |###|###|###| m | m | m | m | m | m | m | m | m | m | m | m | m | m | m |###|###|###|
	// +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
	// |###|###|###| m | m | m | m | m | m | m | m | m | m | m | m | m | m | m |###|###|###|
	// +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
	// |###|###|###| m | m | m | m | m | m | m | m | m | m | m | m | m | m | m |###|###|###|
	// +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
	// |###|###|###| m | m | m | m | m | m | m | m | m | m | m | m | m | m | m |###|###|###|
	// +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
	// |###|###|###| m | m | m | m | m | m | m | m | m | m | m | m | m | m | m |###|###|###|
	// +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
	// |###|###|###| m | m | m | m | m | m | m | m | m | m | m | m | m | m | m |###|###|###|
	// +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
	// |###|###|###| m | m | m | m | m | m | m | m | m | m | m | m | m | m | m |###|###|###|
	// +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
	// |###|###|###| m | m | m | m | m | m | m | m | m | m | m | m | m | m | m |###|###|###|
	// +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
	// |###|###|###| m | m | m | m | m | m | m | m | m | m | m | m | m | m | m |###|###|###|
	// +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
	// |###|###|###|###|###|###|###|###|###|###|###|###|###|###|###|###|###|###|###|###|###|  <--+
	// +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+	   |
	// |###|###|###|###|###|###|###|###|###|###|###|###|###|###|###|###|###|###|###|###|###|	   +---- padding tiles
	// +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+	   |
	// |###|###|###|###|###|###|###|###|###|###|###|###|###|###|###|###|###|###|###|###|###|	<--+
	// +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+




	class MapMatrix
	{
	public:
		void build(const rex::Map* mapObject);

		s32 width_in_blocks() const;
		s32 height_in_blocks() const;
		s32 major_dimension_in_blocks() const;
		s32 width_in_tiles() const;
		s32 height_in_tiles() const;
		s32 major_dimension_in_tiles() const;

		// Returns the block index at the location
		s32 index_at(BlockCoord blockCoord) const;

		// Returns the block index of the block the tile belongs to
		s32 index_at(TileCoord tileCoord) const;

	private:
		void init_blocks(s8 borderBlockIdx);
		void fill_connections(const rex::Map* mapObject);
		void fill_inner_map(const rex::Map* mapObject);

	private:
		rsl::unique_array<u8> m_block_indices;
		s8 m_total_width;
		s8 m_total_height;
	};

	struct TilesetDesc
	{
		const rex::TextureAsset* tileset_texture;
		const rex::Blockset* blockset;

		// Store how big the tiles are in pixels
		s32 tile_width_px;
		s32 tile_height_px;
	};

	struct ViewportWidgetDesc
	{
		// The dimensions of the viewport itself
		s32 width;
		s32 height;
		rsl::pointi8 camera_pos;

		TilesetDesc tileset_desc;
		const Map* map;
	};

	class ViewportWidget : public Widget
	{
	public:
		ViewportWidget(const ViewportWidgetDesc& desc);

	protected:
		bool on_update() override;

	private:
		void process_input();
		void update_renderpass_data();
		void draw_imgui();

	private:
		TilePass* m_render_pass;		// The renderpass used for rendering the viewport

		// As we're rendering tiles, which are grid based
		// we don't need to store a fancy camera
		// All we care about is which tile we start rendering at the top left
		// That'll be the camera position
		rsl::pointi8 m_camera_pos;

		// The render target that the render pass renders to. 
		// It's this render target that get used to render the final image to the viewport
		rsl::unique_ptr<rex::gfx::RenderTarget> m_render_target;	

		

	};
}