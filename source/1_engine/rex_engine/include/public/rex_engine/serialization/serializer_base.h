#pragma once

#include "rex_engine/memory/blob_view.h"

#include "rex_engine/text_processing/json.h"
#include "rex_engine/assets/asset.h"

namespace rex
{
	class Serializer
	{
	public:
		virtual ~Serializer() = default;

		virtual rsl::unique_ptr<Asset> serialize_from_json(const rex::json::json& jsonContent) = 0;
		virtual rsl::unique_ptr<Asset> serialize_from_binary(memory::BlobView content) = 0;

		virtual rex::json::json serialize_to_json(Asset* asset) = 0;
		virtual rex::memory::Blob serialize_to_binary(Asset* asset) = 0;
	};
}