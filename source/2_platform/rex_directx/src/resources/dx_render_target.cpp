#include "rex_directx/resources/dx_render_target.h"

#include "rex_directx/utility/dx_util.h"

namespace rex
{
  namespace gfx
  {
    DxRenderTarget::DxRenderTarget(const wrl::ComPtr<ID3D12Resource>& resource, DxResourceView view, const rsl::Color4f& clearColor)
      : RenderTarget(static_cast<s32>(resource->GetDesc().Width), static_cast<s32>(resource->GetDesc().Height), &m_view, clearColor)
      , m_resource(resource)
      , m_view(view)
    {}

    void DxRenderTarget::debug_set_name(rsl::string_view name)
    {
      d3d::set_debug_name_for(m_resource.Get(), name);
    }

    ID3D12Resource* DxRenderTarget::dx_object()
    {
      return m_resource.Get();
    }

    const DxResourceView& DxRenderTarget::dx_view() const
    {
      return m_view;
    }
  }
}