#include "rex_engine/gfx/system/gpu_cache.h"

#include "rex_engine/gfx/graphics.h"

#include "rex_engine/gfx/shader_reflection/shader_reflection.h"

namespace rex
{
	namespace gfx
	{
		InputLayout* GpuCache::load_input_layout(const InputLayoutDesc& desc)
		{
			auto it = m_input_layout_cache.find(desc);
			if (it != m_input_layout_cache.end())
			{
				return it->value.get();
			}

			auto layout = gfx::gal::instance()->create_input_layout(desc);
			return m_input_layout_cache.emplace(desc, rsl::move(layout)).inserted_element->value.get();
		}
		RootSignature* GpuCache::load_root_signature(const ShaderPipeline& pipeline)
		{
			auto it = m_root_sig_cache.find(pipeline);
			if (it != m_root_sig_cache.end())
			{
				return it->value.get();
			}

			const ShaderPipelineReflection& reflection = shader_reflection::instance()->load(pipeline);
			rsl::unique_ptr<RootSignature> root_sig = gfx::gal::instance()->create_root_signature(reflection.parameters);
			return m_root_sig_cache.emplace(pipeline, rsl::move(root_sig)).inserted_element->value.get();
		}


		namespace gpu_cache
		{
			globals::GlobalUniquePtr<GpuCache> g_gpu_cache;
			void init(globals::GlobalUniquePtr<GpuCache> cache)
			{
				g_gpu_cache = rsl::move(cache);
			}
			GpuCache* instance()
			{
				return g_gpu_cache.get();
			}
			void shutdown()
			{
				g_gpu_cache.reset();
			}
		}
	}
}