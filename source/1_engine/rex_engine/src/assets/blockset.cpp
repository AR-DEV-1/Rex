#include "rex_engine/assets/blockset.h"

namespace rex
{
  Blockset::Blockset(Tileset* tileset, rsl::unique_array<Block> blocks)
    : m_tileset(tileset)
    , m_blocks(rsl::move(blocks))
  {

  }

  const Block& Blockset::block(s32 idx) const
  {
    return m_blocks[idx];
  }
}