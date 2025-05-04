#pragma once

#include "rex_engine/assets/asset.h"

#include "rex_engine/engine/types.h"

namespace rex
{
  namespace gfx
  {
    class Texture2D;
  }

	class Tileset : public Asset
  {
  public:
    //Tileset(const u8* tilesetData);
    Tileset(rsl::unique_ptr<gfx::Texture2D> texture);
    //const u8* tile_pixel_row(u8 tileIdx, s8 pxRow) const;

  private:
    //const u8* tile_data(u8 tileIdx) const;

  private:
    constexpr static s16 s_tileset_width_px = 128;
    constexpr static s16 s_tile_pixel_byte_size = 1;

    //const u8* m_tileset_data;
    rsl::unique_ptr<gfx::Texture2D> m_texture_resource;
  };

  class Tile
  {
  public:
    static constexpr s16 width_px()
    {
      return s_width_px;
    }
    static constexpr s16 height_px()
    {
      return s_height_px;
    }

  private:
    constexpr static s16 s_width_px = 8;
    constexpr static s16 s_height_px = 8;
  };
}