#pragma once

#include "rex_engine/engine/globals.h"

#include "rex_engine/gfx/system/shader_pipeline.h"

#include "rex_engine/gfx/shader_reflection/shader_pipeline_reflection.h"
#include "rex_engine/gfx/shader_reflection/shader_param_declaration.h"
#include "rex_engine/gfx/shader_reflection/shader_signature.h"

#include "rex_engine/gfx/system/shader_parameter_location.h"

#include "rex_engine/gfx/system/shader_parameters_store.h"

#include "rex_std/string.h"
#include "rex_std/unordered_map.h"

namespace rex
{
	namespace gfx
	{
		class ShaderReflectionBase
		{
		public:
			virtual ~ShaderReflectionBase() = default;

			// Load reflection from the cache, create new one if one isn't cached already
			const ShaderPipelineReflection& load(const ShaderPipeline& pipeline);

		protected:
			virtual ShaderSignature reflect(const Shader* shader) = 0;

		private:
			rsl::unordered_map<ShaderPipeline, ShaderPipelineReflection> m_reflection_cache_lookup;
		};

		namespace shader_reflection
		{
			void init(globals::GlobalUniquePtr<ShaderReflectionBase> shaderReflCache);
			ShaderReflectionBase* instance();
			void shutdown();
		}
	}
}