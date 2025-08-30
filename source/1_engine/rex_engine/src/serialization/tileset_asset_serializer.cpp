#include "rex_engine/serialization/tileset_asset_serializer.h"

#include "rex_engine/assets/tileset_asset.h"

#include "rex_engine/engine/asset_db.h"

#include "rex_std/bonus/math.h"

namespace rex
{
	rsl::unique_ptr<Asset> TilesetAssetSerializer::serialize_from_json(const rex::json::json& jsonContent, LoadFlags loadFlags)
	{
		rsl::pointi8 tile_size{};
		tile_size.x = jsonContent["tile_size"]["x"];
		tile_size.y = jsonContent["tile_size"]["y"];

		rsl::string_view tileset = jsonContent["tileset"];
		Tileset* tileset_texture = asset_db::instance()->load<Tileset>(tileset);

		return rsl::make_unique<TilesetAsset>(tile_size, tileset_texture);
	}
	rsl::unique_ptr<Asset> TilesetAssetSerializer::serialize_from_binary(memory::BlobView content)
	{
		return nullptr;
	}

	void TilesetAssetSerializer::hydrate_asset(Asset* asset, const rex::json::json& jsonContent)
	{

	}
	void TilesetAssetSerializer::hydrate_asset(Asset* asset, memory::BlobView content)
	{

	}

	rex::json::json TilesetAssetSerializer::serialize_to_json(Asset* asset)
	{
		return {};
	}
	rex::memory::Blob TilesetAssetSerializer::serialize_to_binary(Asset* asset)
	{
		return {};
	}

}