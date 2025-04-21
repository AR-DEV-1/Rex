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

    void RenderEngine::new_frame()
    {
      m_upload_buffer->reset();
      api_new_frame();
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