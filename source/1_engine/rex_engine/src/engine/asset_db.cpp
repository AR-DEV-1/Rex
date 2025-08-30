#include "rex_engine/engine/asset_db.h"

#include "rex_engine/filesystem/vfs.h"
#include "rex_engine/text_processing/text_processing.h"

#include "rex_engine/event_system/event_system.h"
#include "rex_engine/event_system/events/loading/begin_asset_load.h"
#include "rex_engine/event_system/events/loading/end_asset_load.h"

namespace rex
{
	DEFINE_LOG_CATEGORY(LogAssetDatabase);

	void AssetDb::unload_all()
	{
		m_path_to_asset.clear();
		m_asset_to_metadata.clear();
	}

	rsl::string_view AssetDb::asset_path(const Asset* asset)
	{
		return m_asset_to_metadata.at(asset).path;
	}

	Asset* AssetDb::load_from_json(rsl::type_id_t assetTypeId, rsl::string_view assetPath, LoadFlags loadFlags)
	{
		// If we already have the asset loaded, let's return it 
		Asset* cached_asset = lookup_cached_asset(assetPath, loadFlags);
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

		// We know we can load the asset, so fire the event that it's beginning to load
		event_system::instance()->fire_event(BeginAssetLoad(assetPath));

		// Hydrate the asset if it was partially loaded before
		Asset* potentially_partially_loaded_asset = lookup_cached_asset(assetPath, LoadFlags::PartialLoad);
		if (potentially_partially_loaded_asset)
		{
			m_serializers.at(assetTypeId.name())->hydrate_asset(potentially_partially_loaded_asset, asset_json);
			m_asset_to_metadata.at(potentially_partially_loaded_asset).is_partially_loaded = false;
			return potentially_partially_loaded_asset;
		}

		// Deserialize, initialize and cache the asset
		rsl::unique_ptr<Asset> asset = m_serializers.at(asset_type_name)->serialize_from_json(asset_json, loadFlags);

		AssetMetaData metadata{};
		metadata.path = rsl::string(assetPath);
		metadata.is_partially_loaded = rsl::has_flag(loadFlags, LoadFlags::PartialLoad);
		m_asset_to_metadata.emplace(asset.get(), metadata);
		auto emplace_result = m_path_to_asset.emplace(assetPath, rsl::move(asset));

		// Asset is loaded, so fire the event that is has fully loaded
		event_system::instance()->fire_event(EndAssetLoad(assetPath, asset.get()));

		return emplace_result.inserted_element->value.get();
	}
	Asset* AssetDb::load_from_binary(rsl::type_id_t assetTypeId, rsl::string_view assetPath, LoadFlags loadFlags)
	{
		// If we already have the asset loaded, let's return it 
		Asset* cached_asset = lookup_cached_asset(assetPath, loadFlags);
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

		// If we don't have a (de)serializer for the type, we can't initialize it, so return
		if (!m_serializers.contains(assetTypeId.name()))
		{
			REX_ERROR(LogAssetDatabase, "No serializer added to loaded an asset of type \"{}\"", assetTypeId.name());
			return nullptr;
		}

		// We know we can load the asset, so fire the event that it's beginning to load
		event_system::instance()->fire_event(BeginAssetLoad(assetPath));

		// load the asset from disk
		rex::memory::Blob asset_blob = rex::vfs::instance()->read_file(assetPath);

		// Hydrate the asset if it was partially loaded before
		Asset* potentially_partially_loaded_asset = lookup_cached_asset(assetPath, LoadFlags::PartialLoad);
		if (potentially_partially_loaded_asset)
		{
			m_serializers.at(assetTypeId.name())->hydrate_asset(potentially_partially_loaded_asset, asset_blob);
			m_asset_to_metadata.at(potentially_partially_loaded_asset).is_partially_loaded = false;
			return potentially_partially_loaded_asset;
		}

		// Deserialize, initialize and cache the asset
		rsl::unique_ptr<Asset> asset = m_serializers.at(assetTypeId.name())->serialize_from_binary(asset_blob/*, loadFlags*/);
		AssetMetaData metadata{};
		metadata.path = rsl::string(assetPath);
		metadata.is_partially_loaded = rsl::has_flag(loadFlags, LoadFlags::PartialLoad);
		m_asset_to_metadata.emplace(asset.get(), metadata);
		auto emplace_result = m_path_to_asset.emplace(assetPath, rsl::move(asset));

		// Asset is loaded, so fire the event that is has fully loaded
		event_system::instance()->fire_event(EndAssetLoad(assetPath, asset.get()));
		return emplace_result.inserted_element->value.get();
	}

	Asset* AssetDb::lookup_cached_asset(rsl::string_view assetPath, LoadFlags loadFlags)
	{
		if (!m_path_to_asset.contains(assetPath))
		{
			return nullptr;
		}

		Asset* asset = m_path_to_asset.at(assetPath).get();
		const AssetMetaData& asset_metadata = m_asset_to_metadata.at(asset);
		if (asset_metadata.is_partially_loaded && !rsl::has_flag(loadFlags, LoadFlags::PartialLoad))
		{
			return nullptr;
		}

		return asset;
	}

	void AssetDb::serialize(rsl::type_id_t assetTypeId, Asset* asset, rsl::string_view assetPath)
	{
	}

	void AssetDb::hydrate_asset(rsl::type_id_t assetTypeId, Asset* asset)
	{
		if (!m_asset_to_metadata.contains(asset))
		{
			return;
		}

		// If it's already hydrated
		if (m_asset_to_metadata.at(asset).is_partially_loaded == false)
		{
			return;
		}

		rsl::string_view asset_path = m_asset_to_metadata.at(asset).path;
		rex::memory::Blob asset_blob = rex::vfs::instance()->read_file(asset_path);
		rex::json::json asset_json = rex::json::parse(asset_blob);

		m_serializers.at(assetTypeId.name())->hydrate_asset(asset, asset_json);
		m_asset_to_metadata.at(asset).is_partially_loaded = false;
	}

	bool AssetDb::is_partially_loaded(rsl::string_view assetPath)
	{
		if (!m_path_to_asset.contains(assetPath))
		{
			return nullptr;
		}

		Asset* asset = m_path_to_asset.at(assetPath).get();
		return is_partially_loaded(asset);
	}

	bool AssetDb::is_partially_loaded(const Asset* asset)
	{
		if (!asset)
		{
			return false;
		}

		const AssetMetaData& asset_metadata = m_asset_to_metadata.at(asset);
		return asset_metadata.is_partially_loaded;
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