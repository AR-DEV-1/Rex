#include "rex_engine/gfx/system/root_signature_cache.h"


#include "rex_engine/gfx/graphics.h"

#include "rex_std/functional.h"
#include "rex_std/unordered_map.h"

#include "rex_engine/gfx/shader_reflection/shader_reflection.h"

namespace rex
{
	namespace gfx
	{
			RootSignature* RootSignatureCache::load(const ShaderPipeline& pipeline)
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

			void RootSignatureCache::clear()
			{
				m_root_sig_cache.clear();
			}
		namespace root_signature_cache
		{
			globals::GlobalUniquePtr<RootSignatureCache> g_root_sig_cache;
			void init(globals::GlobalUniquePtr<RootSignatureCache> rootSignatureCache)
			{
				g_root_sig_cache = rsl::move(rootSignatureCache);
			}
			RootSignatureCache* instance()
			{
				return g_root_sig_cache.get();
			}
			void shutdown()
			{
				g_root_sig_cache.reset();
			}
		}
	}
}