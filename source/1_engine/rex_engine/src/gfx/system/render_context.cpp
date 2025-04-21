#include "rex_engine/gfx/system/render_context.h"

#include "rex_engine/gfx/graphics.h"

namespace rex
{
  namespace gfx
  {
    RenderContext::RenderContext(gfx::GraphicsEngine* owningEngine)
      : GraphicsContext(owningEngine, GraphicsEngineType::Render)
    {
    }
    void RenderContext::set_render_target(RenderTarget* colorRenderTarget)
    {
      set_render_target(colorRenderTarget, nullptr);
    }
    void RenderContext::clear_render_target(RenderTarget* renderTarget)
    {
      clear_render_target(renderTarget, nullptr);
    }
    void RenderContext::set_vertex_buffer(VertexBuffer* vb)
    {
      set_vertex_buffer(vb, 0);
    }

    // Reset the engine specific context
    void RenderContext::type_specific_reset(const ContextResetData& resetData)
    {
      // By default we set the current backbuffer as render target
      set_render_target(resetData.current_backbuffer_rt);
      m_shader_visible_srv_heap = resetData.shader_visible_srv_desc_heap;
      m_shader_visible_sampler_heap = resetData.shader_visible_sampler_desc_heap;
    }

    const ResourceView* RenderContext::copy_texture_views_to_shaders(const rsl::vector<const ResourceView*>& views)
    {
      return copy_views(m_shader_visible_srv_heap, views);
    }
    const ResourceView* RenderContext::copy_sampler_views_to_shaders(const rsl::vector<const ResourceView*>& views)
    {
      return copy_views(m_shader_visible_sampler_heap, views);
    }
    const ResourceView* RenderContext::copy_views(ViewHeapType heapType, const rsl::vector<const ResourceView*>& views)
    {
      const ResourceView* gpuViews = gfx::gal::instance()->try_get_gpu_views(views);
      if (gpuViews != nullptr)
      {
        return gpuViews;
      }

      switch (heapType)
      {
      case rex::gfx::ViewHeapType::Texture2D:       return copy_texture_views_to_shaders(views);
      case rex::gfx::ViewHeapType::Sampler:         return copy_sampler_views_to_shaders(views);
      default: break;
      }

      return nullptr;
    }

    const ResourceView* RenderContext::copy_views(ViewHeap* dstHeap, const rsl::vector<const ResourceView*>& views)
    {
      rsl::unique_ptr<ResourceView> gpuView = dstHeap->copy_views(views);
      return gfx::gal::instance()->notify_views_on_gpu(views, rsl::move(gpuView));
    }
  }
}