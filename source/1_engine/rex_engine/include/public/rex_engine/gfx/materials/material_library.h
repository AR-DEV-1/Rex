#pragma once

#include "rex_engine/engine/globals.h"
#include "rex_engine/gfx/materials/material.h"
#include "rex_std/string_view.h"

#include "rex_engine/gfx/resources/blend_state.h"
#include "rex_engine/gfx/resources/raster_state.h"
#include "rex_engine/gfx/resources/depth_stencil_state.h"

#include "rex_engine/text_processing/json.h"

// #TODO: Remaining cleanup of development/Pokemon -> main merge. ID: ASSET LOADING

namespace rex
{
	namespace gfx
	{
		class MaterialLibrary
		{
		public:
			// Load a material from disk, given a filepath
			rsl::unique_ptr<Material> load_material(rsl::string_view filepath);

		private:
			void init_material_parameters(Material* /*material*/, const rex::json::json& jsonBlob);

		};

		namespace material_lib
		{
			void init(globals::GlobalUniquePtr<MaterialLibrary> materialLib);
			MaterialLibrary* instance();
			void shutdown();
		}

	}
}