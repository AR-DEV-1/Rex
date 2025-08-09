#include "regina/scene_serializer.h"

#include "regina/scene.h"

namespace regina
{
	rsl::unique_ptr<rex::Asset> SceneSerializer::serialize_from_json(const rex::json::json& jsonContent, rex::LoadFlags loadFlags)
	{
		rsl::string_view name = jsonContent["name"];

		return rsl::make_unique<Scene>(name);
	}
	rsl::unique_ptr<rex::Asset> SceneSerializer::serialize_from_binary(rex::memory::BlobView content)
	{
		return nullptr;
	}

	rex::json::json SceneSerializer::serialize_to_json(rex::Asset* asset)
	{
		return rex::json::json{};
	}
	rex::memory::Blob SceneSerializer::serialize_to_binary(rex::Asset* asset)
	{
		return rex::memory::Blob{};
	}
}