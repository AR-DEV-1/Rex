#include "rex_engine/serialization/blockset_serializer.h"

#include "rex_engine/memory/blob_reader.h"

#include "rex_engine/assets/block.h"
#include "rex_engine/assets/blockset.h"

#include "rex_engine/engine/asset_db.h"

#include "rex_engine/gfx/graphics.h"
#include "rex_engine/gfx/resources/texture_2d.h"

#include "rex_std/bonus/math.h"

namespace rex
{
	rsl::unique_ptr<Asset> BlocksetSerializer::serialize_from_json(const rex::json::json& jsonContent, LoadFlags loadFlags)
	{
		rsl::string_view tileset_path = jsonContent["tileset"];
		rsl::string_view blockset_path = jsonContent["blockset"];

		TilesetAsset* tileset = asset_db::instance()->load<TilesetAsset>(tileset_path);
		rsl::unique_array<Block> blocks = load_block_indices(blockset_path);

		return rsl::make_unique<Blockset>(tileset, rsl::move(blocks));
	}
	rsl::unique_ptr<Asset> BlocksetSerializer::serialize_from_binary(memory::BlobView content)
	{
		return nullptr;
	}

	void BlocksetSerializer::hydrate_asset(Asset* asset, const rex::json::json& jsonContent)
	{}
	void BlocksetSerializer::hydrate_asset(Asset* asset, memory::BlobView content) 
	{}

	rex::json::json BlocksetSerializer::serialize_to_json(Asset* asset)
	{
		return json::json{};
	}
	rex::memory::Blob BlocksetSerializer::serialize_to_binary(Asset* asset)
	{
		return memory::Blob{};
	}

	rsl::unique_array<Block> BlocksetSerializer::load_block_indices(rsl::string_view blockIndicesPath)
	{
		memory::Blob content = vfs::instance()->read_file(blockIndicesPath);

		s64 num_blocks = content.size() / Block::num_tiles();
		memory::BlobReader reader(content);

		Block::indices_array blob_memory;
		rsl::unique_array<Block> blocks = rsl::make_unique<Block[]>(static_cast<s32>(num_blocks));
		for (s32 block_idx = 0; block_idx < num_blocks; ++block_idx)
		{
			reader.read(blob_memory.data(), blob_memory.size());
			blocks[block_idx] = Block(blob_memory);
		}

		return blocks;
	}

}