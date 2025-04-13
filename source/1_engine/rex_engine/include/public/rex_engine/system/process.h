#pragma once

#include "rex_engine/engine/types.h"
#include "rex_engine/memory/memory_types.h"

namespace rex
{
  namespace current_process
  {
		u32 id();
    scratch_string path();
    void path(char8* buffer, s32 length);
  }
}

#ifdef REX_PLATFORM_WINDOWS
  #include "rex_engine/platform/win/system/win_process.h"
#endif