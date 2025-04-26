#pragma once

#include "rex_std/vector.h"

#include "rex_engine/gfx/shader_reflection/shader_signature.h"
#include "rex_engine/gfx/shader_reflection/shader_pipeline_reflection.h"

namespace rex
{
	namespace gfx
	{
		struct ShaderResources
		{
			rsl::vector<ShaderResourceDeclaration> material_resources;
			rsl::vector<ShaderResourceDeclaration> renderpass_resources;
		};

		// For more information about the shader pipeline and pipeline in general, look at these files
		// - pipeline_state.h
		// - shader_pipeline.h
		// 
		// This class is used to create reflection data of a shader pipeline.
		// The user should process every shader within the pipeline and call the build func at the end
		// some example code on how it should be used
		//
		// ```
		// ShaderPipelineReflectionBuilder builder{};
		// builder.process_shader(reflect(pipeline.vs), ShaderVisibility::Vertex);
		// builder.process_shader(reflect(pipeline.ps), ShaderVisibility::Pixel);
		// 
		// ShaderPipelineReflection reflection = builder.build();
		// ```
		class ShaderPipelineReflectionBuilder
		{
		public:
			// Process a single shader. splitting the resources to different structures
			void process_shader(const ShaderSignature& signature, ShaderVisibility visibility);

			// Build the reflection of the entire shader pipeline and return the result
			ShaderPipelineReflection build();

		private:
			ShaderResources split_resources(const rsl::vector<ShaderResourceDeclaration> resources);
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