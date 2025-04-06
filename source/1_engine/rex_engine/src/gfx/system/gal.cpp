#include "rex_engine/gfx/system/gal.h"

// #TODO: Remaining cleanup of development/Pokemon -> main merge. ID: GRAPHICS

#include "rex_engine/gfx/resources/render_target.h"
#include "rex_engine/gfx/resources/texture_2d.h"

#include "rex_engine/gfx/imgui/imgui_device.h"

#include "rex_engine/engine/defines.h"
#include "rex_engine/diagnostics/log.h"

#include "rex_engine/gfx/graphics.h"
#include "rex_engine/gfx/system/gal.h"
#include "rex_engine/gfx/rendering/swapchain_info.h"

namespace rex
{
	namespace gfx
	{
    DEFINE_LOG_CATEGORY(LogGraphics2);

    // Make sure to not try and initialize any gpu resources in the constructor.
    // The derived class of the gpu engine is responsible for making sure the gpu is ready.
    GALInterface::GALInterface(const OutputWindowUserData& userData)
      : m_render_engine()
      , m_compute_engine()
      , m_copy_engine()
      , m_swapchain()
      , m_max_frames_in_flight(userData.max_frames_in_flight)
      , m_primary_display_handle(userData.primary_display_handle)
      , m_frame_idx(0)
    {
    }

    // Because some objects have a dependency on the gpu engine itself
    // We need to wait for the gpu engine to be constructed, only then 
    // can we initialize the rest of the objects
    void GALInterface::init()
    {
      init_resource_heap();
      init_desc_heaps();
      init_sub_engines();
      init_swapchain();
      init_imgui();
    }

    // Prepare a new frame by incrementing the frame index and clearing the backbuffer
    void GALInterface::new_frame()
    {
      ++m_frame_idx;

      m_render_engine->new_frame();
      m_compute_engine->new_frame();
      m_copy_engine->new_frame();

      auto render_ctx = new_render_ctx(rsl::Nullptr<PipelineState>, "New Frame");
      render_ctx->transition_buffer(current_backbuffer_rt(), ResourceState::RenderTarget);
      render_ctx->clear_render_target(current_backbuffer_rt());
      render_ctx->execute_on_gpu();
    }
    // Present the new frame to the main window
    void GALInterface::present()
    {
      auto render_ctx = new_render_ctx(rsl::Nullptr<PipelineState>, "End Frame");
      render_ctx->transition_buffer(current_backbuffer_rt(), ResourceState::Present);
      render_ctx->execute_on_gpu();

      m_swapchain->present();
    }
    // Finish off the last frame
    void GALInterface::end_frame()
    {
      m_render_engine->end_frame();
      m_compute_engine->end_frame();
      m_copy_engine->end_frame();
    }

    void GALInterface::log_info(const Info& info)
    {
			REX_UNUSED_PARAM(info);

			REX_INFO(LogGraphics2, "Renderer Info - Adaptor: {}", info.adaptor);
			REX_INFO(LogGraphics2, "Renderer Info - Vendor: {}", info.vendor);
			REX_INFO(LogGraphics2, "Renderer Info - API: {}", info.api);
			REX_INFO(LogGraphics2, "Renderer Info - API Version: {}", info.api_version);
			REX_INFO(LogGraphics2, "Renderer Info - Shader Version: {}", info.shader_version);
			REX_INFO(LogGraphics2, "Renderer Info - Driver Version: {}", info.driver_version);
    }

    RenderTarget* GALInterface::current_backbuffer_rt()
    {
      return m_swapchain->current_buffer();
    }

    // Create a new context which is used for copying resources from or to the gpu
    ObjectWithDestructionCallback<CopyContext> GALInterface::new_copy_ctx(PipelineState* pso, rsl::string_view eventName)
    {
      ContextResetData reset_data = create_context_reset_data(pso);

      auto base_ctx = m_copy_engine->new_context(reset_data, eventName);
      return base_ctx.convert<CopyContext>();
    }
    // Create a new context which is used for rendering to render targets
    ObjectWithDestructionCallback<RenderContext> GALInterface::new_render_ctx(PipelineState* pso, rsl::string_view eventName)
    {
      ContextResetData reset_data = create_context_reset_data(pso);

      auto base_ctx = m_render_engine->new_context(reset_data, eventName);
      return base_ctx.convert<RenderContext>();
    }
    // Create a new context which is used for computing data on the gpu
    ObjectWithDestructionCallback<ComputeContext> GALInterface::new_compute_ctx(PipelineState* pso, rsl::string_view eventName)
    {
      ContextResetData reset_data = create_context_reset_data(pso);

      auto base_ctx = m_compute_engine->new_context(reset_data, eventName);
      return base_ctx.convert<ComputeContext>();
    }

    // Return the render target pointing to the current backbuffer of the swapchain
    //RenderTarget* GALInterface::current_backbuffer_rt()
    //{
    //  return m_swapchain->current_buffer();
    //}

    void GALInterface::notify_textures_presence_on_gpu(Texture2D* texture, rsl::unique_ptr<ResourceView> resourceView)
    {
      m_textures_on_gpu[texture] = rsl::move(resourceView);
    }
    const ResourceView* GALInterface::try_get_texture_gpu_handle(Texture2D* texture) const
    {
      if (m_textures_on_gpu.contains(texture))
      {
        return m_textures_on_gpu.at(texture).get();
      }

      return nullptr;
    }

    const ResourceView* GALInterface::try_get_gpu_views(const rsl::vector<const ResourceView*>& views) const
    {
      // Hash all pointers together to calculate the final hash
      u64 seed = 0;
      for (const ResourceView* cpuView : views)
      {
        seed = rsl::hash_combine(seed, rsl::comp_hash(cpuView));
      }

      if (m_resources_on_gpu.contains(seed))
      {
        return m_resources_on_gpu.at(seed).get();
      }

      return nullptr;
    }
    const ResourceView* GALInterface::try_get_gpu_view(const ResourceView* cpuView) const
    {
      return try_get_gpu_views({ cpuView });
    }
    const ResourceView* GALInterface::notify_views_on_gpu(const rsl::vector<const ResourceView*>& views, rsl::unique_ptr<ResourceView> gpuView)
    {
      u64 seed = 0;
      for (const ResourceView* cpuView : views)
      {
        seed = rsl::hash_combine(seed, rsl::comp_hash(cpuView));
      }

      const ResourceView* result = gpuView.get();
      m_resources_on_gpu[seed] = rsl::move(gpuView);

      return result;
    }

    // Returns a specific descriptor heap based on type
    ViewHeap* GALInterface::cpu_desc_heap(ViewHeapType descHeapType)
    {
      return m_cpu_descriptor_heap_pool.at(descHeapType).get();
    }
    // Returns a specific descriptor heap based on type that's visible to shaders
    ViewHeap* GALInterface::shader_visible_desc_heap(ViewHeapType descHeapType)
    {
      return m_shader_visible_descriptor_heap_pool.at(descHeapType).get();
    }

    // Initialize the swapchain which is used for presenting to the main window
    void GALInterface::init_swapchain()
    {
      m_swapchain = gfx::instance()->create_swapchain(m_render_engine->command_queue(), m_max_frames_in_flight, m_primary_display_handle);

      SwapchainInfo swapchain_info{};
      swapchain_info.width = m_swapchain->width();
      swapchain_info.height = m_swapchain->height();
      swapchain_info.viewport.top_left_x = 0.0f;
      swapchain_info.viewport.top_left_y = 0.0f;
      swapchain_info.viewport.width = static_cast<f32>(swapchain_info.width);
      swapchain_info.viewport.height = static_cast<f32>(swapchain_info.height);
      swapchain_info.scissor_rect.right = static_cast<f32>(swapchain_info.width);
      swapchain_info.scissor_rect.bottom = static_cast<f32>(swapchain_info.height);

      update_swapchain_info(swapchain_info);
    }
    // Initialize the sub engine, bringing them up and ready, to be used in the graphics pipeline
    void GALInterface::init_sub_engines()
    {
      m_render_engine = init_render_engine(&m_resource_state_tracker);
      m_copy_engine = init_copy_engine(&m_resource_state_tracker);
      m_compute_engine = init_compute_engine(&m_resource_state_tracker);

      m_render_engine->init();
      m_copy_engine->init();
      m_compute_engine->init();
    }
    // Initialize imgui so it can create its own windows if necessary
    void GALInterface::init_imgui()
    {
      ImGuiDevice imgui_device{};
      imgui_device.command_queue = m_render_engine->command_queue();
      imgui_device.max_num_frames_in_flight = m_max_frames_in_flight;
      imgui_device.rtv_format = m_swapchain->format();
      init_imgui_device(imgui_device);
    }
    // Initialize the descriptor heaps which keep track of all descriptors to various resources
    void GALInterface::init_desc_heaps()
    {
      init_desc_heap(m_cpu_descriptor_heap_pool, ViewHeapType::RenderTarget, IsShaderVisible::no);
      init_desc_heap(m_cpu_descriptor_heap_pool, ViewHeapType::DepthStencil, IsShaderVisible::no);
      init_desc_heap(m_cpu_descriptor_heap_pool, ViewHeapType::Texture2D, IsShaderVisible::no);
      init_desc_heap(m_cpu_descriptor_heap_pool, ViewHeapType::Sampler, IsShaderVisible::no);

      init_desc_heap(m_shader_visible_descriptor_heap_pool, ViewHeapType::Texture2D, IsShaderVisible::yes);
      init_desc_heap(m_shader_visible_descriptor_heap_pool, ViewHeapType::Sampler, IsShaderVisible::yes);
    }
    void GALInterface::init_desc_heap(ViewHeapPool& descHeapPool, ViewHeapType descHeapType, IsShaderVisible isShaderVisible)
    {
      descHeapPool.emplace(descHeapType, allocate_view_heap(descHeapType, isShaderVisible));
    }

    ContextResetData GALInterface::create_context_reset_data(PipelineState* pso)
    {
      ContextResetData reset_data{};
      reset_data.pso = pso;
      reset_data.shader_visible_srv_desc_heap = shader_visible_desc_heap(ViewHeapType::Texture2D);
      reset_data.shader_visible_sampler_desc_heap = shader_visible_desc_heap(ViewHeapType::Sampler);
      reset_data.current_backbuffer_rt = m_swapchain->current_buffer();

      return reset_data;
    }

























		//globals::GlobalUniquePtr<GALInterface> g_gal;

    //GALInterface* gfx::instance()
    //{
    //  return g_gal.get();
    //}

    // Initializes the render hardware infrastructure
		// For DirectX, creates the dxgi factory, d3d device, command buffers, heaps and swapchain
		// After this, the gal is setup to start creating resources (textures, shaders, vertex buffers, ..)
		//gfx::GALInterface* init_gal(globals::GlobalUniquePtr<GALInterface> galInterface, const OutputWindowUserData& userData)
		//{
		//	REX_ASSERT_X(g_gal == nullptr, "The graphics abstraction layer has already been assigned, this is not allowed");
		//	g_gal = rsl::move(galInterface);
		//	return g_gal->create_gpu_engine(userData);
		//}

		// shutdown the internal gal, all reference to the gal are invalidated from here on out
		//void shutdown_gal()
		//{
		//	g_gal.reset();
		//}
	}
}