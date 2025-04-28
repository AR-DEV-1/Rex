#pragma once

#include "rex_engine/engine/types.h"
#include "rex_engine/gfx/resources/resource.h"
#include "rex_engine/gfx/resources/clear_state.h"
#include "rex_engine/gfx/system/resource_view.h"

namespace rex
{
  namespace gfx
  {
    // A render target resource, only holding its width and height
    // The actual resource is implemented in an API specific class
    class RenderTarget : public Resource
    {
    public:
      RenderTarget(s32 width, s32 height, ResourceView* resourceView, const rsl::Color4f& clearColor);
      virtual ~RenderTarget() = default;

      s32 width() const;
      s32 height() const;
      
      const rsl::Color4f& clear_color() const;

      // Return the resource view of the texture
      const ResourceView* view() const;

    private:
      s32 m_width;
      s32 m_height;
      rsl::Color4f m_clear_color;
      ResourceView* m_view;
    };
  }
}