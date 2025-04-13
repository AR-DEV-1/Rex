#include "rex_engine/gfx/graphics.h"



#include "rex_engine/gfx/core/gpu_description.h"
#include "rex_engine/gfx/system/root_signature_cache.h"

#include "rex_std/bonus/utility.h"
#include "rex_std/memory.h"

#include "rex_engine/gfx/resources/render_target.h"
#include "rex_engine/gfx/resources/texture_2d.h"

#include "rex_engine/gfx/imgui/imgui_device.h"

#include "rex_engine/engine/defines.h"
#include "rex_engine/diagnostics/log.h"

#include "rex_engine/gfx/graphics.h"

#include "rex_engine/gfx/rendering/swapchain_info.h"

// #TODO: Remaining cleanup of development/Pokemon -> main merge. ID: GRAPHICS

namespace rex
{
	namespace gfx
	{
		DEFINE_LOG_CATEGORY(LogGraphics);

		namespace gal
		{

			globals::GlobalUniquePtr<GALBase> g_gal_interface;
			Error init(globals::GlobalUniquePtr<GALBase> galInterface)
			{
				rsl::scopeguard shutdown_on_failure = []() { shutdown(); };

				g_gal_interface = rsl::move(galInterface);
				g_gal_interface->init();

				// Make sure we release the scopeguard which would shutdown our graphics systems otherwise
				shutdown_on_failure.release();
				return Error::no_error();
			}
			GALBase* instance()
			{
				return g_gal_interface.get();
			}
			void shutdown()
			{
				root_signature_cache::clear();

				g_gal_interface.reset();
			}
		}



		// Make sure to not try and initialize any gpu resources in the constructor.
		// The derived class of the gpu engine is responsible for making sure the gpu is ready.
		GALBase::GALBase(const OutputWindowUserData& userData)
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
		void GALBase::init()
		{
			api_init();

			// Log the info in case anything goes wrong after this.
			log_info(info());

			init_resource_heap();
			init_desc_heaps();
			init_sub_engines();
			init_swapchain();
			init_imgui();

			// Init the common resources so that we don't have to repeat creating the same resources
			init_common_resources();
		}

		void GALBase::render()
		{
			// The render loop is very simple
			// - Prepare a new frame
			// - Loop over all the renderers
			// - Finalize the frame
			// - Present

			// Prepare all resources that are needed for a new frame
			new_frame();

			// Loop over all renderers and call into them
			// Performing all required gpu operations needed on resources those renderers need
			// The resources are owned by the renderers, they're responsible for creating and destroying them
			// However in the backend, the gpu manager controls when these resources actually get deallocated
			for (auto& renderer : m_renderers)
			{
				renderer->render();
			}

			// Finalize the frame and bringing it ready for presentation
			end_frame();

			// Present the last rendered frame to the screen
			present();
		}

		// Prepare a new frame by incrementing the frame index and clearing the backbuffer
		void GALBase::new_frame()
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
		void GALBase::present()
		{
			auto render_ctx = new_render_ctx(rsl::Nullptr<PipelineState>, "End Frame");
			render_ctx->transition_buffer(current_backbuffer_rt(), ResourceState::Present);
			render_ctx->execute_on_gpu();

			m_swapchain->present();
		}
		// Finish off the last frame
		void GALBase::end_frame()
		{
			m_render_engine->end_frame();
			m_compute_engine->end_frame();
			m_copy_engine->end_frame();
		}

		void GALBase::log_info(const Info& info)
		{
			REX_UNUSED_PARAM(info);

			REX_INFO(LogGraphics, "Renderer Info - Adaptor: {}", info.adaptor);
			REX_INFO(LogGraphics, "Renderer Info - Vendor: {}", info.vendor);
			REX_INFO(LogGraphics, "Renderer Info - API: {}", info.api);
			REX_INFO(LogGraphics, "Renderer Info - API Version: {}", info.api_version);
			REX_INFO(LogGraphics, "Renderer Info - Shader Version: {}", info.shader_version);
			REX_INFO(LogGraphics, "Renderer Info - Driver Version: {}", info.driver_version);
		}

		// Return the current render target of the swapchain
		RenderTarget* GALBase::swapchain_rt()
		{
			return m_swapchain->current_buffer();
		}
		// Return the width of the render target of the swapchain
		s32 GALBase::back_buffer_width()
		{
			return m_swapchain->width();
		}
		// Return the height of the render target of the swapchain
		s32 GALBase::back_buffer_height()
		{
			return m_swapchain->height();
		}
		RenderTarget* GALBase::current_backbuffer_rt()
		{
			return m_swapchain->current_buffer();
		}

		// Create a new context which is used for copying resources from or to the gpu
		ObjectWithDestructionCallback<CopyContext> GALBase::new_copy_ctx(PipelineState* pso, rsl::string_view eventName)
		{
			ContextResetData reset_data = create_context_reset_data(pso);

			auto base_ctx = m_copy_engine->new_context(reset_data, eventName);
			return base_ctx.convert<CopyContext>();
		}
		// Create a new context which is used for rendering to render targets
		ObjectWithDestructionCallback<RenderContext> GALBase::new_render_ctx(PipelineState* pso, rsl::string_view eventName)
		{
			ContextResetData reset_data = create_context_reset_data(pso);

			auto base_ctx = m_render_engine->new_context(reset_data, eventName);
			return base_ctx.convert<RenderContext>();
		}
		// Create a new context which is used for computing data on the gpu
		ObjectWithDestructionCallback<ComputeContext> GALBase::new_compute_ctx(PipelineState* pso, rsl::string_view eventName)
		{
			ContextResetData reset_data = create_context_reset_data(pso);

			auto base_ctx = m_compute_engine->new_context(reset_data, eventName);
			return base_ctx.convert<ComputeContext>();
		}

		void GALBase::notify_textures_presence_on_gpu(Texture2D* texture, rsl::unique_ptr<ResourceView> resourceView)
		{
			m_textures_on_gpu[texture] = rsl::move(resourceView);
		}
		const ResourceView* GALBase::try_get_texture_gpu_handle(Texture2D* texture) const
		{
			if (m_textures_on_gpu.contains(texture))
			{
				return m_textures_on_gpu.at(texture).get();
			}

			return nullptr;
		}

		const ResourceView* GALBase::try_get_gpu_views(const rsl::vector<const ResourceView*>& views) const
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
		const ResourceView* GALBase::try_get_gpu_view(const ResourceView* cpuView) const
		{
			return try_get_gpu_views({ cpuView });
		}
		const ResourceView* GALBase::notify_views_on_gpu(const rsl::vector<const ResourceView*>& views, rsl::unique_ptr<ResourceView> gpuView)
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

		RasterStateDesc GALBase::common_raster_state(CommonRasterState type)
		{
			return m_common_raster_states.at(rsl::enum_refl::enum_index(type).value());

		}
		Sampler2D* GALBase::common_sampler(CommonSampler type)
		{
			return m_common_samplers.at(rsl::enum_refl::enum_index(type).value()).get();
		}

		// Returns a specific descriptor heap based on type
		ViewHeap* GALBase::cpu_desc_heap(ViewHeapType descHeapType)
		{
			return m_cpu_descriptor_heap_pool.at(descHeapType).get();
		}
		// Returns a specific descriptor heap based on type that's visible to shaders
		ViewHeap* GALBase::shader_visible_desc_heap(ViewHeapType descHeapType)
		{
			return m_shader_visible_descriptor_heap_pool.at(descHeapType).get();
		}

		// Initialize the swapchain which is used for presenting to the main window
		void GALBase::init_swapchain()
		{
			m_swapchain = gfx::gal::instance()->create_swapchain(m_render_engine->command_queue(), m_max_frames_in_flight, m_primary_display_handle);

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
		void GALBase::init_sub_engines()
		{
			m_render_engine = init_render_engine(&m_resource_state_tracker);
			m_copy_engine = init_copy_engine(&m_resource_state_tracker);
			m_compute_engine = init_compute_engine(&m_resource_state_tracker);

			m_render_engine->init();
			m_copy_engine->init();
			m_compute_engine->init();
		}
		// Initialize imgui so it can create its own windows if necessary
		void GALBase::init_imgui()
		{
			ImGuiDevice imgui_device{};
			imgui_device.command_queue = m_render_engine->command_queue();
			imgui_device.max_num_frames_in_flight = m_max_frames_in_flight;
			imgui_device.rtv_format = m_swapchain->format();
			init_imgui_device(imgui_device);
		}
		// Initialize the descriptor heaps which keep track of all descriptors to various resources
		void GALBase::init_desc_heaps()
		{
			init_desc_heap(m_cpu_descriptor_heap_pool, ViewHeapType::RenderTarget, IsShaderVisible::no);
			init_desc_heap(m_cpu_descriptor_heap_pool, ViewHeapType::DepthStencil, IsShaderVisible::no);
			init_desc_heap(m_cpu_descriptor_heap_pool, ViewHeapType::Texture2D, IsShaderVisible::no);
			init_desc_heap(m_cpu_descriptor_heap_pool, ViewHeapType::Sampler, IsShaderVisible::no);

			init_desc_heap(m_shader_visible_descriptor_heap_pool, ViewHeapType::Texture2D, IsShaderVisible::yes);
			init_desc_heap(m_shader_visible_descriptor_heap_pool, ViewHeapType::Sampler, IsShaderVisible::yes);
		}
		void GALBase::init_desc_heap(ViewHeapPool& descHeapPool, ViewHeapType descHeapType, IsShaderVisible isShaderVisible)
		{
			descHeapPool.emplace(descHeapType, allocate_view_heap(descHeapType, isShaderVisible));
		}

		void GALBase::init_common_resources()
		{
			// common raster states
			RasterStateDesc raster_state{};
			raster_state.fill_mode = rex::gfx::FillMode::Solid;
			raster_state.cull_mode = rex::gfx::CullMode::Back;
			raster_state.front_ccw = false;
			raster_state.depth_clip_enable = true;
			m_common_raster_states[rsl::enum_refl::enum_index(CommonRasterState::DefaultDepth).value()] = raster_state;

			raster_state.depth_clip_enable = false;
			m_common_raster_states[rsl::enum_refl::enum_index(CommonRasterState::DefaultNoDepth).value()] = raster_state;

			// common samplers
			SamplerDesc sampler_desc{};
			sampler_desc.filtering = rex::gfx::SamplerFiltering::MinMagMipPoint;
			sampler_desc.address_mode_u = rex::gfx::TextureAddressMode::Wrap;
			sampler_desc.address_mode_v = rex::gfx::TextureAddressMode::Wrap;
			sampler_desc.address_mode_w = rex::gfx::TextureAddressMode::Wrap;
			sampler_desc.mip_lod_bias = 0.0f;
			sampler_desc.max_anisotropy = 0;
			sampler_desc.comparison_func = rex::gfx::ComparisonFunc::Always;
			sampler_desc.border_color = rex::gfx::BorderColor::TransparentBlack;
			sampler_desc.min_lod = 0.0f;
			sampler_desc.max_lod = 0.0f;
			sampler_desc.shader_register = 0;
			sampler_desc.register_space = 0;
			sampler_desc.shader_visibility = rex::gfx::ShaderVisibility::Pixel;
			m_common_samplers[rsl::enum_refl::enum_index(CommonSampler::Default2D).value()] = gfx::gal::instance()->create_sampler2d(sampler_desc);
		}

		ContextResetData GALBase::create_context_reset_data(PipelineState* pso)
		{
			ContextResetData reset_data{};
			reset_data.pso = pso;
			reset_data.shader_visible_srv_desc_heap = shader_visible_desc_heap(ViewHeapType::Texture2D);
			reset_data.shader_visible_sampler_desc_heap = shader_visible_desc_heap(ViewHeapType::Sampler);
			reset_data.current_backbuffer_rt = m_swapchain->current_buffer();

			return reset_data;
		}
	}
}