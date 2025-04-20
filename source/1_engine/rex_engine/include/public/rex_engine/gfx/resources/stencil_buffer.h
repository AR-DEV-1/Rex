#pragma once

#include "rex_engine/engine/types.h"
#include "rex_engine/gfx/core/texture_format.h"
#include "rex_engine/gfx/resources/resource.h"
#include "rex_engine/gfx/resources/clear_state.h"

namespace rex
{
	namespace gfx
	{
		class ResourceView;

		// A stencil buffer is a buffer holding stencil data
		// Main purpose of this is to be used for stencil testing
		// which is used to determine if you want to draw certain pixels or not
		class StencilBuffer : public Resource
		{
		public:
			StencilBuffer(ResourceView* view, s32 width, s32 height, TextureFormat format, const ClearStateDesc& clearStateDesc);

			// Return the width of the stencil buffer
			s32 width() const;
			// Return the height of the stencil buffer
			s32 height() const;
			// Return the format of the stencil buffer
			TextureFormat format() const;
			// Return the resource view of the stencil buffer
			ResourceView* resource_view();
			// Return the clear state of the stencil buffer
			const ClearStateDesc& clear_state() const;

		private:
			s32 m_width;
			s32 m_height;
			TextureFormat m_format;
			ResourceView* m_view;
			ClearStateDesc m_clear_state;
		};
	}
}