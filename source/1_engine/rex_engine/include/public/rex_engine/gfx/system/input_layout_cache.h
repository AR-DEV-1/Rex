#pragma once

#include "rex_engine/gfx/resources/input_layout.h"

#include "rex_engine/engine/globals.h"

#include "rex_std/unordered_map.h"
#include "rex_std/memory.h"

namespace rex
{
	namespace gfx
	{
		class InputLayoutCache
		{
		public:
			// Load an input layout from the cache, or store a new one
			InputLayout* load(const InputLayoutDesc& desc);
			// Clear the cache
			void clear();

		private:
			rsl::unordered_map<InputLayoutDesc, rsl::unique_ptr<InputLayout>> m_cached_layouts;
		};

		// Input layouts don't need to be created anymore in recent graphics APIs
		// However we often need to keep the graphics API specific data around
		// That's why we'll cache these
		namespace input_layout_cache
		{
			void init(globals::GlobalUniquePtr<InputLayoutCache> inputLayoutCache);
			InputLayoutCache* instance();
			void shutdown();
		}
	}
}