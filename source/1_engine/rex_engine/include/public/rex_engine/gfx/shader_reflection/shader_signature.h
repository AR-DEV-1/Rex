#pragma once

#include "rex_engine/gfx/system/shader_type.h"

#include "rex_engine/gfx/shader_reflection/shader_resource_declaration.h"
#include "rex_engine/gfx/shader_reflection/constant_buffer_declaration.h"
#include "rex_engine/gfx/shader_reflection/shader_io_declaration.h"

#include "rex_engine/gfx/system/shader_pipeline.h"

namespace rex
{
	namespace gfx
	{
		// Holds a list of resources that are needed for a resource
		struct ShaderResourceDeclarations
		{
			rsl::vector<ShaderResourceDeclaration> byte_address_buffers;  // Holds all the unordered access buffers of the shader
			rsl::vector<ShaderResourceDeclaration> unordered_access_buffers;  // Holds all the unordered access buffers of the shader
			rsl::vector<ShaderResourceDeclaration> constant_buffers;  // Holds all the constant buffers of the shader
			rsl::vector<ShaderResourceDeclaration> textures;					// Holds all the textures of the shader
			rsl::vector<ShaderResourceDeclaration> samplers;					// Holds all the samplers of the shader
		};
		struct ShaderSignatureDesc
		{
			rsl::vector<ConstantBufferDeclaration> constant_buffers;			// Holds all the constant buffers of the shader
			rsl::vector<ShaderIODeclaration>   input_params;	// Holds all the input parameters of the shader
			rsl::vector<ShaderIODeclaration>   output_params;	// Holds all the output parameters of the shader
			ShaderResourceDeclarations bound_resources;											// Holds all bound resources of the shader
			rsl::tiny_stack_string shader_version;							// The version of the shader
			ShaderType type;																		// The type of the shader
		};

		// A signature describes all the resources that need to be bound for a shader
		class ShaderSignature
		{
		public:
			ShaderSignature() = default;
			ShaderSignature(ShaderSignatureDesc&& desc);

			// The shader type this signature belongs to
			ShaderType shader_type() const;

			// The constant buffers of the shader
			const rsl::vector<ConstantBufferDeclaration>& constant_buffers() const;
			// The input parameters of the shader
			const rsl::vector<ShaderIODeclaration>& input_params() const;
			// The output parameters of the shader
			const rsl::vector<ShaderIODeclaration>& output_params() const;
			// The byte address buffers of the shader in resource format
			const rsl::vector<ShaderResourceDeclaration>& byte_address_buffers_resources() const;
			// The unordered access buffers of the shader in resource format
			const rsl::vector<ShaderResourceDeclaration>& unordered_access_buffers_resources() const;
			// The constant buffers of the shader in resource format
			const rsl::vector<ShaderResourceDeclaration>& constant_buffers_resources() const;
			// The textures of the shader
			const rsl::vector<ShaderResourceDeclaration>& textures() const;
			// The samplers of the shader
			const rsl::vector<ShaderResourceDeclaration>& samplers() const;

		private:
			ShaderSignatureDesc m_desc;
		};
	}
}