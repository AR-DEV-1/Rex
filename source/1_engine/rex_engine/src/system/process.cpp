#include "rex_engine/system/process.h"
#include "rex_engine/filesystem/path.h"

namespace rex
{
	namespace current_process
	{
		scratch_string path()
		{
			path_stack_string filepath;
			path(filepath.data(), filepath.max_size());
			filepath.reset_null_termination_offset();

			return scratch_string(filepath);
		}
	}
}