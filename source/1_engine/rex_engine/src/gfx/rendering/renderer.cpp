#include "rex_engine/gfx/rendering/renderer.h"

#include "rex_engine/gfx/graphics.h"
#include "rex_engine/gfx/rendering/render_pass.h"
#include "rex_engine/gfx/system/shader_library.h"

#include "rex_engine/filesystem/path.h"

#include "rex_std/bonus/math.h"

namespace rex
{
	namespace gfx
	{
		Renderer::Renderer()
		{
		}

		namespace renderer
		{
			globals::GlobalUniquePtr<Renderer> g_renderer;
			void init(globals::GlobalUniquePtr<Renderer> renderer)
			{
				g_renderer = rsl::move(renderer);
			}
			Renderer* instance()
			{
				return g_renderer.get();
			}
			void shutdown()
			{
				g_renderer.reset();
			}
		}
	}
}