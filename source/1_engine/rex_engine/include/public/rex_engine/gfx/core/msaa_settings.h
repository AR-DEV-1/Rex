#pragma once

#include "rex_engine/engine/types.h"

namespace rex
{
  namespace gfx
  {
    // graphics api abstraction of multi sampling settings
    struct MsaaSettings
    {
      s32 num_samples_per_pixel   = 1; // number of multi samples per pixel
      s32 quality = 0; // the image quality level (max value is GPU specific and should always get queried)
    };
  }                    // namespace gfx
} // namespace rex