#pragma once

#include "rex_engine/memory/blob_view.h"

#include "rex_engine/text_processing/json.h"
#include "rex_engine/assets/asset.h"

namespace rex
{
	enum class LoadFlags
	{
		None,
		PartialLoad = BIT(0) // The serializer is reponsible for the partial loading
	};

	class Serializer
	{
	public:
		virtual ~Serializer() = default;

		// Deserialize an asset from json or binary and return the new asset
		// The asset can be partially loaded, requiring full hydration at a later date
		virtual rsl::unique_ptr<Asset> serialize_from_json(const rex::json::json& jsonContent, LoadFlags loadFlags = LoadFlags::None) = 0;
		virtual rsl::unique_ptr<Asset> serialize_from_binary(memory::BlobView content) = 0;

		// Hydrate an already loaded asset. This will always use the existing pointer of an asset and never reallocate the pointer
		// This does not mean that no reallocation ever takes place
		// Internally during hydration, it's possible internal members of the asset reallocate
		// that is up to the serializer
		virtual void hydrate_asset(Asset* asset, const rex::json::json& jsonContent) = 0;
		virtual void hydrate_asset(Asset* asset, memory::BlobView content) = 0;

		// Serialize an asset to json or binary representation
		virtual rex::json::json serialize_to_json(Asset* asset) = 0;
		virtual rex::memory::Blob serialize_to_binary(Asset* asset) = 0;
	};
}