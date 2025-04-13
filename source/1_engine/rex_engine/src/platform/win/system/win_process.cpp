#include "rex_engine/platform/win/system/win_process.h"
#include "rex_engine/filesystem/path.h"

#include <Windows.h>

namespace rex
{
	namespace current_process
	{
		rsl::win::handle_t handle()
		{
			return GetCurrentProcess();
		}
		u32 id()
		{
			return static_cast<u32>(GetProcessId(handle()));
		}
		void path(char8* buffer, s32 length)
		{
			GetModuleFileNameA(NULL, buffer, length);
		}
	}
} // namespace rex