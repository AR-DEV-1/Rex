#include "rex_engine/serialization/texture_serializer.h"

#include "rex_engine/assets/texture_asset.h"

#include "rex_engine/serialization/image_loading.h"

namespace rex
{
	rsl::unique_ptr<Asset> TextureSerializer::serialize_from_json(const rex::json::json& jsonContent)
	{
		REX_ASSERT("Textures cannot be loaded from json as they are binary assets");
		return nullptr;
	}
	rsl::unique_ptr<Asset> TextureSerializer::serialize_from_binary(memory::BlobView content)
	{
		ImageLoadResult image_load_res = load_image(content);
		return rsl::make_unique<TextureAsset>(rsl::move(image_load_res.data), image_load_res.width, image_load_res.height, image_load_res.num_channels);
	}

	rex::json::json TextureSerializer::serialize_to_json(Asset* asset)
	{
		return json::json{};
	}
	rex::memory::Blob TextureSerializer::serialize_to_binary(Asset* asset)
	{
		return memory::Blob{};
	}
}