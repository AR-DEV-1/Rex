#pragma once

#include "rex_engine/engine/globals.h"

#include "rex_engine/assets/asset.h"
#include "rex_engine/serialization/serializer_base.h"
#include "rex_engine/filesystem/vfs.h"
#include "rex_engine/filesystem/path.h"

#include "rex_std/bonus/algorithms.h"
#include "rex_std/bonus/utility.h"
#include "rex_std/string_view.h"
#include "rex_std/unordered_map.h"

namespace rex
{
	// Asset dependency tracking should not be done in asset DB
	struct AssetMetaData
	{
		rsl::string path;
		bool is_partially_loaded;
	};

	class AssetDb
	{
	public:
		// Load an asset from disk
		// The extension is used to determine if it the asset is saved as json or binary
		template <typename T>
		T* load(rsl::string_view assetPath, LoadFlags loadFlags = LoadFlags::None)
		{
			if (path::extension(assetPath) == ".json")
			{
				return load_from_json<T>(assetPath, loadFlags);
			}
			else
			{
				return load_from_binary<T>(assetPath, loadFlags);
			}
		}

		// Save an asset to the location where it was read from
		template <typename T>
		void save(T* asset)
		{
			save(asset, m_asset_to_metadata.at(asset).path);
		}

		// Save an asset to a given filepath
		template <typename T>
		void save(T* asset, rsl::string_view assetPath)
		{
			scratch_string fullpath = rex::vfs::instance()->abs_path(assetPath);
			fullpath.replace("\\", "/");
			rsl::to_lower(fullpath.cbegin(), fullpath.begin(), fullpath.length());
			serialize(rsl::type_id<T>(), asset, assetPath);
		}

		// Hydrate a previously loaded asset, filling it with data
		template <typename T>
		void hydra_asset(T* asset)
		{
			hydra_asset(rsl::type_id<T>(), asset);
		}

		template <typename T>
		void add_serializer(rsl::unique_ptr<Serializer> serializer)
		{
			rsl::string_view type_name = rsl::type_id<T>().name();
			REX_ASSERT_X(!m_serializers.contains(type_name), "A serializer is already present for {}", type_name);
			m_serializers.emplace(type_name, rsl::move(serializer));
		}

		void unload_all();

		rsl::string_view asset_path(const Asset* asset);

	private:
		template <typename T>
		T* load_from_json(rsl::string_view assetPath, LoadFlags loadFlags = LoadFlags::None)
		{
			scratch_string fullpath = rex::vfs::instance()->abs_path(assetPath);
			fullpath.replace("\\", "/");
			rsl::to_lower(fullpath.cbegin(), fullpath.begin(), fullpath.length());
			return static_cast<T*>(load_from_json(rsl::type_id<T>(), fullpath, loadFlags));
		}

		template <typename T>
		T* load_from_binary(rsl::string_view assetPath, LoadFlags loadFlags = LoadFlags::None)
		{
			scratch_string fullpath = rex::vfs::instance()->abs_path(assetPath);
			fullpath.replace("\\", "/");
			rsl::to_lower(fullpath.cbegin(), fullpath.begin(), fullpath.length());
			return static_cast<T*>(load_from_binary(rsl::type_id<T>(), fullpath, loadFlags));
		}

		Asset* load_from_json(rsl::type_id_t assetTypeId, rsl::string_view assetPath, LoadFlags loadFlags);
		Asset* load_from_binary(rsl::type_id_t assetTypeId, rsl::string_view assetPath, LoadFlags loadFlags);
		Asset* lookup_cached_asset(rsl::string_view assetPath, LoadFlags loadFlags);

		void serialize(rsl::type_id_t assetTypeId, Asset* asset, rsl::string_view assetPath);

		void hydra_asset(rsl::type_id_t assetTypeId, Asset* asset);
		bool is_partially_loaded(rsl::string_view assetPath);
		bool is_partially_loaded(const Asset* asset);

	private:
		rsl::unordered_map<rsl::string_view, rsl::unique_ptr<Serializer>> m_serializers;
		rsl::unordered_map<rsl::string, rsl::unique_ptr<Asset>> m_path_to_asset;
		rsl::unordered_map<const Asset*, AssetMetaData> m_asset_to_metadata;
	};

	namespace asset_db
	{
		void init(globals::GlobalUniquePtr<AssetDb> assetDb);
		AssetDb* instance();
		void shutdown();
	}
}