#include "rex_engine/serialization/tileset_serializer.h"

#include "rex_engine/assets/tileset.h"
#include "rex_engine/filesystem/vfs.h"
#include "rex_engine/engine/asset_db.h"
#include "rex_engine/assets/texture_asset.h"

#include "rex_engine/serialization/image_loading.h"

#include "rex_std/bonus/math.h"

#include "rex_engine/gfx/graphics.h"
#include "rex_engine/gfx/resources/texture_2d.h"

namespace rex
{
	rsl::unique_ptr<Asset> TilesetSerializer::serialize_from_json(const rex::json::json& jsonContent)
	{
		REX_ASSERT("Tilesets cannot be loaded from json as they are binary assets");
		return nullptr;
	}
	rsl::unique_ptr<Asset> TilesetSerializer::serialize_from_binary(memory::BlobView content)
	{
		ImageLoadResult tileset_img_load_res = load_image(content);

		// A tileset only holds 1 channel, we have to convert it to 4 channels as that's what the GPU expects
		rsl::unique_array<rsl::Rgba> tileset_rgba = rsl::make_unique<rsl::Rgba[]>(tileset_img_load_res.width * tileset_img_load_res.height * sizeof(rsl::Rgba));
		for (s32 color_idx = 0; color_idx < tileset_img_load_res.width * tileset_img_load_res.height; ++color_idx)
		{
			u8 color = tileset_img_load_res.data[color_idx];
			rsl::Rgba& rgba = tileset_rgba[color_idx];
			rgba.red = color;
			rgba.green = color;
			rgba.blue = color;
			rgba.alpha = 255;
		}

		rsl::unique_ptr<rex::gfx::Texture2D> texture = rex::gfx::gal::instance()->create_texture2d(tileset_img_load_res.width, tileset_img_load_res.height, rex::gfx::TextureFormat::Unorm4, tileset_rgba.get());
		return rsl::make_unique<Tileset>(rsl::move(texture));
	}

	rex::json::json TilesetSerializer::serialize_to_json(Asset* asset)
	{
		return json::json{};
	}
	rex::memory::Blob TilesetSerializer::serialize_to_binary(Asset* asset)
	{
		return memory::Blob{};
	}
}