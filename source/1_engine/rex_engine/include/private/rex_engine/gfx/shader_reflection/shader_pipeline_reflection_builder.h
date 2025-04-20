#pragma once

#include "rex_std/vector.h"

#include "rex_engine/gfx/shader_reflection/shader_signature.h"
#include "rex_engine/gfx/shader_reflection/shader_pipeline_reflection.h"

namespace rex
{
	namespace gfx
	{
		struct SplittedResources
		{
			rsl::vector<ShaderResourceDeclaration> material_resources;
			rsl::vector<ShaderResourceDeclaration> renderpass_resources;
		};

		class ShaderPipelineReflectionBuilder
		{
		public:
			void process_shader(const ShaderSignature& signature, ShaderVisibility visibility);

			ShaderPipelineReflection build();

		private:
			SplittedResources split_resources(const rsl::vector<ShaderResourceDeclaration> resources);
			void add_bindings(const rsl::vector<ShaderResourceDeclaration>& resources, ShaderParameterType type, s32 expectedRegisterSpace, ShaderVisibility visibility);
			void add_view_binding(ShaderParametersStoreDesc* paramStoreDesc, const rsl::vector<ShaderResourceDeclaration>& resources, ShaderParameterType type, s32 expectedRegisterSpace, ShaderVisibility visibility);
			void add_view_table_binding(ShaderParametersStoreDesc* paramStoreDesc, const rsl::vector<ShaderResourceDeclaration>& resources, ShaderParameterType type, s32 expectedRegisterSpace, ShaderVisibility visibility);

		private:
			ShaderPipelineReflection m_reflection_result;

			static constexpr s32 s_renderpass_register_space = 0;
			static constexpr s32 s_material_register_space = 1;
		};
	}
}