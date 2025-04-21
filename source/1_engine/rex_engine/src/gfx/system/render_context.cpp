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
    
    // Set the render target on the pipeline, removes any depth stencil buffer from the pipeline
    void RenderContext::set_render_target(RenderTarget* colorRenderTarget)
    {
      set_render_target(colorRenderTarget, nullptr);
    }
    // Clears the render target with its clear color. Does not clear depth stencil buffer
    void RenderContext::clear_render_target(RenderTarget* renderTarget)
    {
      clear_render_target(renderTarget, nullptr);
    }
    // Assigns a vertex buffer to the pipeline at slot 0
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

    // Copy views of a certain type to the gpu. All views within the list are expected to be of the same type
    const ResourceView* RenderContext::copy_views(ResourceViewType resourceType, const rsl::vector<const ResourceView*>& views)
    {
      const ResourceView* gpuViews = gfx::gal::instance()->try_get_gpu_views(views);
      if (gpuViews != nullptr)
      {
        return gpuViews;
      }

      switch (resourceType)
      {
      case rex::gfx::ResourceViewType::Texture2D:       return copy_views(m_shader_visible_srv_heap, views);
      case rex::gfx::ResourceViewType::Sampler:         return copy_views(m_shader_visible_sampler_heap, views);
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