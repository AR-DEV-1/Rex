#pragma once

#include "rex_directx/system/dx_view_heap.h"

#include "rex_directx/system/dx_resource_heap.h"
#include "rex_directx/utility/dx_util.h"
#include "rex_engine/engine/types.h"

#include "rex_engine/memory/memory_types.h"
#include "rex_engine/gfx/system/swapchain.h"

#include <dxgi1_4.h>

namespace rex
{
  namespace gfx
  {
    struct OutputWindowUserData;
    struct MsaaSettings;

    class DxSwapchain : public Swapchain
    {
    public:
      DxSwapchain(const wrl::ComPtr<IDXGISwapChain3>& swapchain, s32 width, s32 height, DXGI_FORMAT format, s32 bufferCount);

      // Swap the current back buffer with the front buffer
      void present() override;

      // Return the internal wrapped swapchain object
      IDXGISwapChain3* dx_object();

      // Return the index of the current back buffer
      s32 current_buffer_idx() const override;

      // Resize the backbuffers to a new resolution
      void resize(s32 newWidth, s32 newHeight) override;

    private:
      // Query the buffers from the swapped swapchain object and cache them
      void query_buffers(s32 bufferCount);

      // Retarget the render targets to point to their new buffers and update their internal render target view
      void retarget_buffers(const scratch_vector<DxResourceView>& rtvs);

    private:
      wrl::ComPtr<IDXGISwapChain3> m_swapchain;
      DXGI_FORMAT m_format;
    };
  } // namespace gfx
} // namespace rex