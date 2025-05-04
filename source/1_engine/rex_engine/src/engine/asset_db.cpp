#include "rex_engine/engine/asset_db.h"

#include "rex_engine/filesystem/vfs.h"
#include "rex_engine/text_processing/text_processing.h"

namespace rex
{
	DEFINE_LOG_CATEGORY(LogAssetDatabase);

	void AssetDb::unload_all()
	{
		m_path_to_asset.clear();
	}

	Asset* AssetDb::load_from_json(rsl::type_id_t assetTypeId, rsl::string_view assetPath)
	{
		// If we already have the asset loaded, let's return it 
		Asset* cached_asset = lookup_cached_asset(assetPath);
		if (cached_asset)
		{
			return cached_asset;
		}

		// If the file doesn't exist, we can't load it
		if (!rex::vfs::instance()->exists(assetPath))
		{
			REX_ERROR(LogAssetDatabase, "asset at path {} does not exist", quoted(assetPath));
			return nullptr;
		}

		REX_VERBOSE(LogAssetDatabase, "Loading {}", assetPath);

		// load the asset from disk
		rex::memory::Blob asset_blob = rex::vfs::instance()->read_file(assetPath);
		rex::json::json asset_json = rex::json::parse(asset_blob);

		// If the json content could not be parsed, we can't continue, so we return
		if (asset_json.is_discarded())
		{
			REX_ERROR(LogAssetDatabase, "invalid json for asset {}", quoted(assetPath));
			return nullptr;
		}

		// If the asset is not of the expect type, we error out here as we can't initialize it
		rsl::string_view asset_type_name = asset_json["type_name"];
		if (asset_type_name != assetTypeId.name())
		{
			REX_ERROR(LogAssetDatabase, "Asset at {} is not of expected type. Expecting {}, actual {}", quoted(assetPath), assetTypeId.name(), asset_type_name);
			return nullptr;
		}
		
		// If we don't have a (de)serializer for the type, we can't initialize it, so return
		if (!m_serializers.contains(asset_type_name))
		{
			REX_ERROR(LogAssetDatabase, "No serializer added to loaded an asset of type {}", asset_type_name);
			return nullptr;
		}

		// Deserialize, initialize and cache the asset
		rsl::unique_ptr<Asset> asset = m_serializers.at(asset_type_name)->serialize_from_json(asset_json);
		auto emplace_result = m_path_to_asset.emplace(assetPath, rsl::move(asset));

		return emplace_result.inserted_element->value.get();
	}
	Asset* AssetDb::load_from_binary(rsl::type_id_t assetTypeId, rsl::string_view assetPath)
	{
		// If we already have the asset loaded, let's return it 
		Asset* cached_asset = lookup_cached_asset(assetPath);
		if (cached_asset)
		{
			return cached_asset;
		}

		// If the file doesn't exist, we can't load it
		if (!rex::vfs::instance()->exists(assetPath))
		{
			REX_ERROR(LogAssetDatabase, "asset at path {} does not exist", quoted(assetPath));
			return nullptr;
		}

		REX_VERBOSE(LogAssetDatabase, "Loading {}", assetPath);

		// load the asset from disk
		rex::memory::Blob asset_blob = rex::vfs::instance()->read_file(assetPath);

		// If we don't have a (de)serializer for the type, we can't initialize it, so return
		if (!m_serializers.contains(assetTypeId.name()))
		{
			REX_ERROR(LogAssetDatabase, "No serializer added to loaded an asset of type {}", assetTypeId.name());
			return nullptr;
		}

		// Deserialize, initialize and cache the asset
		rsl::unique_ptr<Asset> asset = m_serializers.at(assetTypeId.name())->serialize_from_binary(asset_blob);
		auto emplace_result = m_path_to_asset.emplace(assetPath, rsl::move(asset));

		return emplace_result.inserted_element->value.get();
	}

	Asset* AssetDb::lookup_cached_asset(rsl::string_view assetPath)
	{
		if (!m_path_to_asset.contains(assetPath))
		{
			return nullptr;
		}

		return m_path_to_asset.at(assetPath).get();;
	}

	namespace asset_db
	{
		globals::GlobalUniquePtr<AssetDb> g_asset_db;
		void init(globals::GlobalUniquePtr<AssetDb> assetDb)
		{
			g_asset_db = rsl::move(assetDb);
		}
		AssetDb* instance()
		{
			return g_asset_db.get();
		}
		void shutdown()
		{
			g_asset_db.reset();
		}
	}
}