#pragma once

#include "rex_engine/serialization/serializer_base.h"

namespace regina
{
	class SceneSerializer : public rex::Serializer
	{
	public:
		rsl::unique_ptr<rex::Asset> serialize_from_json(const rex::json::json& jsonContent, rex::LoadFlags loadFlags) override;
		rsl::unique_ptr<rex::Asset> serialize_from_binary(rex::memory::BlobView content) override;

		void hydrate_asset(rex::Asset* asset, const rex::json::json& jsonContent) override;
		void hydrate_asset(rex::Asset* asset, rex::memory::BlobView content) override;

		rex::json::json serialize_to_json(rex::Asset* asset) override;
		rex::memory::Blob serialize_to_binary(rex::Asset* asset) override;

	};
}