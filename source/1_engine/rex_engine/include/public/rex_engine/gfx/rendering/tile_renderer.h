#pragma once

#include "rex_engine/gfx/resources/texture_2d.h"
#include "rex_engine/gfx/resources/render_target.h"

namespace rex
{
	namespace gfx
	{
		// Note: perhaps this can be a render pass instead and doesn't need a dedicated renderer?

		struct TileRendererDesc
		{
			// The width in pixels of a single tile
			s32 tile_width_px;
			// The height in pixels of a single tile
			s32 tile_height_px;

			// The tileset texture
			// this texture will be used to index into to get a tile
			const Texture2D* tileset_texture;
		};

		class TilesRenderPass
		{
		public:
			TilesRenderPass(const TileRendererDesc& desc);

			const RenderTarget* rendered_output() const;

		private:
			TileRendererDesc m_desc;
		};

		// The tile renderer is used to render 2D tiles on screen
		// It's implementation is based on how tiles were rendered in the original Pokemon games
		// To render a tile you need a few pieces of data
		// - the tile index within a tileset
		class TileRenderer
		{
		public:
			TileRenderer(const TileRendererDesc& desc);

		private:
			const Texture2D* m_tileset_texture;
		};
	}
}