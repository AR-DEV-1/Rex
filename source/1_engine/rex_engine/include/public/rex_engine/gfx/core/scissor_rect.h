#pragma once

#include "rex_engine/engine/types.h"

namespace rex
{
  namespace gfx
  {
    // Graphics api abstraction of the scissor rec
    // A scissor rect is the region within the viewport you're allowed to draw
    // If you specify the viewports dimension, you draw to the viewport
    struct ScissorRect
    {
      ScissorRect()
        : left(0.0f)
        , top(0.0f)
        , right(0.0f)
        , bottom(0.0f)
      {
      }

      ScissorRect(f32 l, f32 t, f32 r, f32 b)
        : left(l)
        , top(t)
        , right(r)
        , bottom(b)
      {
      }

      f32 left;
      f32 top;
      f32 right;
      f32 bottom;
    };
  }
} // namespace rex