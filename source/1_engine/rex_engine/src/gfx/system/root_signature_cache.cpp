#include "rex_engine/gfx/system/root_signature_cache.h"


#include "rex_engine/gfx/graphics.h"

#include "rex_std/functional.h"
#include "rex_std/unordered_map.h"

#include "rex_engine/gfx/shader_reflection/shader_reflection_cache.h"

namespace rex
{
	namespace gfx
	{
		namespace root_signature_cache
		{
			rsl::unordered_map<ShaderPipeline, rsl::unique_ptr<RootSignature>> g_root_sig_cache;

			RootSignature* load(const ShaderPipeline& pipeline)
			{
				auto it = g_root_sig_cache.find(pipeline);
				if (it != g_root_sig_cache.end())
				{
					return it->value.get();
				}

				const ShaderPipelineReflection& reflection = shader_reflection_cache::load(pipeline);
				rsl::unique_ptr<RootSignature> root_sig = gfx::gal::instance()->create_root_signature(reflection.parameters);
				return g_root_sig_cache.emplace(pipeline, rsl::move(root_sig)).inserted_element->value.get();
			}

			void clear()
			{
				g_root_sig_cache.clear();
			}
		}
	}
}