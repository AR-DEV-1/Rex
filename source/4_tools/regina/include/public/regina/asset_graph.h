#pragma once

#include "rex_std/string.h"
#include "rex_std/string_view.h"
#include "rex_std/unordered_map.h"
#include "rex_std/vector.h"
#include "rex_engine/assets/asset.h"

namespace regina
{
	class AssetGraph
	{
	public:
		AssetGraph();

	private:
		void on_begin_asset_load(rsl::string_view assetPath);
		void on_end_asset_load(rsl::string_view assetPath, const rex::Asset* asset);

	private:
		rsl::unordered_map<rsl::string, rsl::vector<rsl::string>> m_asset_dependencies;
		rsl::vector<rsl::string_view> m_asset_dependency_stack;
	};
}