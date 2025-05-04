#include "regina/scene_manager.h"

#include "rex_engine/filesystem/vfs.h"

#include "rex_engine/engine/asset_db.h"
#include "rex_engine/diagnostics/log.h"
#include "rex_engine/text_processing/text_processing.h"
#include "rex_engine/text_processing/json.h"

namespace regina
{
	DEFINE_LOG_CATEGORY(LogSceneManager);

	void SceneManager::load_scene(rsl::string_view scenePath)
	{
		if (!rex::vfs::instance()->exists(scenePath))
		{
			REX_ERROR(LogSceneManager, "scene with path {} does not exist", rex::quoted(scenePath));
			return;
		}

		REX_INFO(LogSceneManager, "Loading {}", scenePath);

		m_active_scene = rex::asset_db::instance()->load_from_json<rex::Map>(scenePath);
	}
}