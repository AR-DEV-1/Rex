#pragma once

#include "rex_engine/assets/asset.h"

#include "rex_engine/engine/types.h"
#include "rex_engine/assets/tileset_asset.h"
#include "rex_engine/assets/block.h"

#include "rex_std/memory.h"
#include "rex_std/bonus/math.h"

namespace rex
{
	class Tileset;

	class Blockset : public Asset
	{
	public:
		Blockset(TilesetAsset* tileset, rsl::unique_array<Block> blocks);

		const Block& block(s32 idx) const;
		const TilesetAsset* tileset() const;

	private:
		TilesetAsset* m_tileset;
		rsl::unique_array<Block> m_blocks;
	};
}