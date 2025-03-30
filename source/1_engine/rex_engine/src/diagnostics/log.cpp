#include "rex_engine/diagnostics/log.h"

#include "rex_engine/filesystem/path.h"
#include "rex_engine/filesystem/vfs.h"

#include "rex_std/format.h"

// #TODO: Remaining cleanup of development/Pokemon -> main merge. ID: LOG PROJECT PATH

namespace rex
{
	scratch_string project_log_path()
	{
		return path::join(engine::instance()->current_session_root(), "logs", rsl::format("{}.log", engine::instance()->project_name()));
	}
}