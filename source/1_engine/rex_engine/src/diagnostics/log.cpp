#include "rex_engine/diagnostics/log.h"

#include "rex_engine/filesystem/path.h"
#include "rex_engine/filesystem/vfs.h"

#include "rex_std/format.h"

namespace rex
{
	rsl::stack_string<char8, 256> project_log_path()
	{
		static rsl::stack_string<char8, 256> res(rsl::string_view(path::join(engine::instance()->current_session_root(), "logs", rsl::format("{}.log", engine::instance()->project_name()))));
		return res;
	}
}