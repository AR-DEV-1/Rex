#include "rex_engine/gfx/system/input_layout_cache.h"

#include "rex_engine/gfx/graphics.h"


#include "rex_std/functional.h"
#include "rex_std/unordered_map.h"

namespace rex
{
	namespace gfx
	{
		InputLayout* InputLayoutCache::load(const InputLayoutDesc& desc)
		{
			auto it = m_cached_layouts.find(desc);
			if (it != m_cached_layouts.end())
			{
				return it->value.get();
			}

			auto layout = gfx::gal::instance()->create_input_layout(desc);
			return m_cached_layouts.emplace(desc, rsl::move(layout)).inserted_element->value.get();
		}

		void InputLayoutCache::clear()
		{
			m_cached_layouts.clear();
		}

		namespace input_layout_cache
		{
			globals::GlobalUniquePtr<InputLayoutCache> g_input_layout_cache;
			void init(globals::GlobalUniquePtr<InputLayoutCache> inputLayoutCache)
			{
				g_input_layout_cache = rsl::move(inputLayoutCache);
			}
			InputLayoutCache* instance()
			{
				return g_input_layout_cache.get();
			}
			void shutdown()
			{
				g_input_layout_cache.reset();
			}
		}
	}
}