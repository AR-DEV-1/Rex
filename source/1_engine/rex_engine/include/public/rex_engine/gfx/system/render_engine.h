#pragma once

#include "rex_engine/gfx/system/graphics_engine.h"
#include "rex_engine/gfx/system/render_context.h"
#include "rex_engine/gfx/system/upload_buffer_lock.h"
#include "rex_engine/gfx/resources/upload_buffer.h"



namespace rex
{
  namespace gfx
  {
    struct PerFrameBackbufferInfo
    {
      RenderTarget* render_target;
      Viewport viewport;
      ScissorRect scissor_rect;
    };

    // The base graphics engine type used for submitting render commands
    class RenderEngine : public GraphicsEngine
    {
    public:
      RenderEngine(ResourceStateTracker* globalResourceStateTracker);

      void new_frame(const PerFrameBackbufferInfo& backbufferInfo);
      void end_frame();

      RenderTarget* current_backbuffer();
      const Viewport& swapchain_viewport() const;
      const ScissorRect& swapchain_scissor_rect() const;

      UploadBufferLock lock_upload_buffer();
      void unlock_upload_buffer();

    protected:
      virtual void api_new_frame() = 0;
      virtual void api_end_frame() = 0;

    private:
      rsl::unique_ptr<UploadBuffer> m_upload_buffer;
      rsl::mutex m_upload_buffer_access_mtx;
      PerFrameBackbufferInfo m_backbuffer_info;
    };
  }
}