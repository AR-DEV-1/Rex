#pragma once

#include "rex_engine/serialization/serializer_base.h"

namespace rex
{
	class Block;

	class BlocksetSerializer : public Serializer
	{
	public:
		rsl::unique_ptr<Asset> serialize_from_json(const rex::json::json& jsonContent, LoadFlags loadFlags) override;
		rsl::unique_ptr<Asset> serialize_from_binary(memory::BlobView content) override;

		void hydrate_asset(Asset* asset, const rex::json::json& jsonContent) override;
		void hydrate_asset(Asset* asset, memory::BlobView content) override;

		rex::json::json serialize_to_json(Asset* asset) override;
		rex::memory::Blob serialize_to_binary(Asset* asset) override;

	private:
		rsl::unique_array<Block> load_block_indices(rsl::string_view blockIndicesPath);
	};
}