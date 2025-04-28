#include "rex_engine/gfx/system/render_engine.h"

#include "rex_engine/gfx/graphics.h"

namespace rex
{
  namespace gfx
  {
    RenderEngine::RenderEngine(ResourceStateTracker* globalResourceStateTracker)
      : GraphicsEngine(GraphicsEngineType::Render, globalResourceStateTracker)
    {
      m_upload_buffer = gfx::gal::instance()->create_upload_buffer(5_mib);
    }

    void RenderEngine::new_frame(const PerFrameBackbufferInfo& backbufferInfo)
    {
      m_upload_buffer->reset();
      m_backbuffer_info = backbufferInfo;
      api_new_frame();
    }
    void RenderEngine::end_frame()
    {
      api_end_frame();
    }

    RenderTarget* RenderEngine::current_backbuffer()
    {
      return m_backbuffer_info.render_target;
    }
    const Viewport& RenderEngine::swapchain_viewport() const
    {
      return m_backbuffer_info.viewport;
    }
    const ScissorRect& RenderEngine::swapchain_scissor_rect() const
    {
      return m_backbuffer_info.scissor_rect;
    }

    UploadBufferLock RenderEngine::lock_upload_buffer()
    {
      m_upload_buffer_access_mtx.lock();
      return UploadBufferLock(this, m_upload_buffer.get());
    }
    void RenderEngine::unlock_upload_buffer()
    {
      m_upload_buffer_access_mtx.unlock();
    }
  }
}