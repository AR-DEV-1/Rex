#include "rex_directx/resources/dx_render_target.h"

#include "rex_directx/utility/dx_util.h"

namespace rex
{
  namespace gfx
  {
    DxRenderTarget::DxRenderTarget(const wrl::ComPtr<ID3D12Resource>& resource, DxResourceView view, const ClearStateDesc& clearState)
      : RenderTarget(static_cast<s32>(resource->GetDesc().Width), static_cast<s32>(resource->GetDesc().Height), &m_view, clearState)
      , m_resource(resource)
      , m_view(view)
    {}

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