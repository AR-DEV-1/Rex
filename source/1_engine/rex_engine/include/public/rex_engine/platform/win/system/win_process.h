#pragma once

#include "rex_engine/engine/types.h"
#include "rex_engine/platform/win/win_types.h"
#include "rex_std/bonus/platform/windows/handle.h"

#include "rex_engine/system/process.h"

namespace rex
{
  namespace current_process
  {
    rsl::win::handle_t handle();
  }
} // namespace rex