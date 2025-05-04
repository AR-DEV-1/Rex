#pragma once

#include "rex_engine/assets/map.h"

#include "rex_std/string_view.h"

#include "rex_engine/memory/blob.h"

namespace regina
{
	class Scene;

	class SceneManager
	{
	public:
		void load_scene(rsl::string_view scenePath);

	private:
		void parse_scene_blob(const rex::memory::Blob& sceneBlob);

	private:
		rex::Map* m_active_scene;
	};
}