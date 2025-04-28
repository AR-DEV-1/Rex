#include "rex_engine/gfx/shader_reflection/shader_reflection.h"

#include "rex_std/bonus/utility.h"

#include "rex_engine/gfx/shader_reflection/shader_signature.h"
#include "rex_engine/gfx/shader_reflection/shader_resource_declaration.h"
#include "rex_engine/gfx/shader_reflection/shader_pipeline_reflection_builder.h"
#include "rex_engine/gfx/core/shader_visibility.h"

namespace rex
{
	namespace gfx
	{
		const ShaderPipelineReflection& ShaderReflectionBase::load(const ShaderPipeline& pipeline)
		{
			if (m_reflection_cache_lookup.contains(pipeline))
			{
				return m_reflection_cache_lookup.at(pipeline);
			}

			ShaderPipelineReflectionBuilder builder{};
			builder.process_shader(reflect(pipeline.vs), ShaderVisibility::Vertex);
			builder.process_shader(reflect(pipeline.ps), ShaderVisibility::Pixel);

			ShaderPipelineReflection reflection = builder.build();
			auto result = m_reflection_cache_lookup.emplace(pipeline, rsl::move(reflection));
			return result.inserted_element->value;
		}

		namespace shader_reflection
		{
			globals::GlobalUniquePtr<ShaderReflectionBase> g_shader_refl_cache;
			void init(globals::GlobalUniquePtr<ShaderReflectionBase> shaderReflCache)
			{
				g_shader_refl_cache = rsl::move(shaderReflCache);
			}
			ShaderReflectionBase* instance()
			{
				return g_shader_refl_cache.get();
			}
			void shutdown()
			{
				g_shader_refl_cache.reset();
			}
		}
	}
}
