#pragma once

#include "rex_engine/engine/state_controller.h"
#include "rex_engine/gfx/core/clear_bits.h"
#include "rex_std/bonus/math/color.h"

namespace rex
{
  namespace gfx
  {
    // A clear state is to help the GPU to clear a resource
    // If you clear a resource using the clear state you initialized it with
    // that'll go quicker than if you use some other values
    struct ClearStateDesc
    {
      ClearStateDesc()
          : rgba(1.0f, 1.0f, 1.0f, 1.0f)
          , depth(1.0f)
          , stencil(0x00)
          , flags()
      {
      }

      rsl::Color4f rgba;                    // the color to clear the resource with
      f32 depth;                            // the value to set to the depth buffer
      u8 stencil;                           // the value to set to the stencil buffer
      StateController<ClearBits> flags {};  // flags controlloing which buffer we're clearing
    };

    ClearStateDesc default_rtv_clear_state();
    ClearStateDesc default_depth_clear_state();
    ClearStateDesc default_stencil_clear_state();
    ClearStateDesc default_dsv_clear_state();
  } // namespace gfx
} // namespace rex