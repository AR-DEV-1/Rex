#include "rex_engine/gfx/resources/sampler_2d.h"

namespace rex
{
	namespace gfx
	{
		Sampler2D::Sampler2D(ResourceView* view)
			: m_view(view)
		{}

		const ResourceView* Sampler2D::resource_view() const
		{
			return m_view;
		}

	}
}