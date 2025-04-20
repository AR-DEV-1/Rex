#pragma once

#include "rex_engine/engine/types.h"

#include "glm/glm.hpp"

namespace rex
{
  namespace gfx
  {
    // Graphics api abstraction of the viewport
    struct Viewport
    {
      Viewport()
        : top_left()
        , width(0.0f)
        , height(0.0f)
        , min_depth(0.0f)
        , max_depth(0.0f)
      {
      }

      Viewport(glm::vec2 topLeft, f32 vpWith, f32 vpHeight, f32 minDepth, f32 maxDepth)
        : top_left(topLeft)
        , width(vpWith)
        , height(vpHeight)
        , min_depth(minDepth)
        , max_depth(maxDepth)
      {
      }

      glm::vec2 top_left;

      f32 width;
      f32 height;

      f32 min_depth;
      f32 max_depth;
    };
  }

} // namespace rex