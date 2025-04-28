#pragma once

namespace rex
{
  enum class TargetPlatform
  {
    Windows
  };
} // namespace rex

#ifdef REX_PLATFORM_WINDOWS

  #include "rex_engine/platform/win/win_platform.h"

#endif
