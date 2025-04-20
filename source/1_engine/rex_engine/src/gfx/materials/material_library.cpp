#include "rex_engine/gfx/materials/material_library.h"

#include "rex_engine/filesystem/file.h"
#include "rex_engine/filesystem/vfs.h"

#include "rex_engine/text_processing/text_processing.h"
#include "rex_engine/text_processing/ini.h"
#include "rex_engine/diagnostics/log.h"

#include "rex_engine/gfx/system/shader_pipeline.h"
#include "rex_engine/gfx/graphics.h"

#include "rex_engine/gfx/shader_reflection/shader_signature.h"
#include "rex_engine/gfx/system/shader_library.h"

#include "rex_engine/string/stringid.h"

#include "rex_engine/gfx/shader_reflection/shader_resource_reflection.h"
#include "rex_std/string.h"

namespace rex
{
	namespace gfx
	{
		DEFINE_LOG_CATEGORY(LogMaterialSystem);

		rsl::unique_ptr<Material> MaterialLibrary::load_material(rsl::string_view filepath)
		{
			REX_DEBUG(LogMaterialSystem, "Loading material {}", quoted(filepath));

			// Try to load the material content from disk
			if (!vfs::instance()->exists(filepath))
			{
				REX_ERROR(LogMaterialSystem, "Cannot load material as it doesn't exists: {}", quoted(filepath));
				return nullptr;
			}

			// Extract its content so that it's processable
			// Materials are saved in json format, so we use a json parser
			rex::memory::Blob file_content = vfs::instance()->read_file(filepath);
			auto json_blob = rex::json::parse(file_content);

			// If the json blob is discarded, that means an error occurred during json parsing
			if (json_blob.is_discarded())
			{
				REX_WARN(LogMaterialSystem, "Failed to process material at {}.", filepath);
				return nullptr;
			}

			// A vertex and pixel shader is mandatory for each material
			REX_ASSERT_X(json_blob.contains("vertex_shader"), "No vertex shader found in material, this is not allowed. Path: {}", quoted(filepath));
			REX_ASSERT_X(json_blob.contains("pixel_shader"), "No pixel shader found in material, this is not allowed. Path: {}", quoted(filepath));

			rsl::string_view vertex_shader = json_blob["vertex_shader"].get<rsl::string_view>();
			rsl::string_view pixel_shader = json_blob["pixel_shader"].get<rsl::string_view>();

			// Process material content so we can create a material object out of it
			MaterialDesc mat_desc{};
			mat_desc.shader_pipeline.vs = shader_lib::instance()->load(vertex_shader, ShaderType::Vertex);
			mat_desc.shader_pipeline.ps = shader_lib::instance()->load(pixel_shader, ShaderType::Pixel);

			// Create the material object
			rsl::unique_ptr<Material> material = gfx::gal::instance()->create_material(mat_desc);

			// Load in the parameters values from the material
			init_material_parameters(material.get(), json_blob); // infinite loop here when inserting into json

			return material;
		}
		void MaterialLibrary::init_material_parameters(Material* /*material*/, const rex::json::json& jsonBlob)
		{
			rsl::string_view parameters_key = "parameters";
			if (!jsonBlob.contains(parameters_key))
			{
				return;
			}

			const auto& parameters_blob = jsonBlob[parameters_key];

			for (const auto& param : parameters_blob)
			{
				rsl::string_view type = param["type"];
				rsl::hash_result type_hash = rsl::comp_hash(type);

				ShaderParameterType param_type = ShaderParameterType::Undefined;
				switch (type_hash)
				{
				case "texture"_sid: param_type = ShaderParameterType::Texture; break;
				case "sampler"_sid: param_type = ShaderParameterType::Sampler; break;
				default: REX_ASSERT("Invalid material param type {}", quoted(type)); break;
				}

				rsl::string_view name = param["name"];
				rsl::string_view path = param["path"];

				// Currently won't work as the resources created should not return unique ptrs as its lifetime should be managed elsewhere
				// However, to get around the compiler errors, we implement it as this for now
				switch (param_type)
				{
				case rex::gfx::ShaderParameterType::Texture: break; // material->set_texture(name, gfx::gal::instance()->create_texture2d(path).get());
				case rex::gfx::ShaderParameterType::Sampler: break; // material->set_sampler(name, gfx::gal::instance()->create_sampler2d(path).get());
				default: break;
				}
			}
		}

		namespace material_lib
		{
			globals::GlobalUniquePtr<MaterialLibrary> g_material_lib;
			void init(globals::GlobalUniquePtr<MaterialLibrary> materialLib)
			{
				g_material_lib = rsl::move(materialLib);
			}
			MaterialLibrary* instance()
			{
				return g_material_lib.get();
			}
			void shutdown()
			{
				g_material_lib.reset();
			}
		}

	}
}