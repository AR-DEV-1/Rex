#pragma once

#include "rex_engine/engine/globals.h"

#include "rex_engine/assets/asset.h"
#include "rex_engine/serialization/serializer_base.h"
#include "rex_engine/filesystem/vfs.h"

#include "rex_std/bonus/utility.h"
#include "rex_std/string_view.h"
#include "rex_std/unordered_map.h"

namespace rex
{
	class AssetDb
	{
	public:
		template <typename T>
		T* load_from_json(rsl::string_view assetPath)
		{
			scratch_string fullpath = rex::vfs::instance()->abs_path(assetPath);
			return static_cast<T*>(load_from_json(rsl::type_id<T>(), fullpath));
		}

		template <typename T>
		T* load_from_binary(rsl::string_view assetPath)
		{
			scratch_string fullpath = rex::vfs::instance()->abs_path(assetPath);
			return static_cast<T*>(load_from_binary(rsl::type_id<T>(), fullpath));
		}

		template <typename T>
		void add_serializer(rsl::unique_ptr<Serializer> serializer)
		{
			rsl::string_view type_name = rsl::type_id<T>().name();
			m_serializers.emplace(type_name, rsl::move(serializer));
		}

		void unload_all();

	private:
		Asset* load_from_json(rsl::type_id_t assetTypeId, rsl::string_view assetPath);
		Asset* load_from_binary(rsl::type_id_t assetTypeId, rsl::string_view assetPath);
		Asset* lookup_cached_asset(rsl::string_view assetPath);

	private:
		rsl::unordered_map<rsl::string_view, rsl::unique_ptr<Serializer>> m_serializers;
		rsl::unordered_map<rsl::string_view, rsl::unique_ptr<Asset>> m_path_to_asset;
	};

	namespace asset_db
	{
		void init(globals::GlobalUniquePtr<AssetDb> assetDb);
		AssetDb* instance();
		void shutdown();
	}
}