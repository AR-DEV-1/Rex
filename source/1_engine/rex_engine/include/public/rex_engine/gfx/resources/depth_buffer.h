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

		// A depth buffer is a buffer holding depth data
		// Main purpose of this is to be used for depth testing
		class DepthBuffer : public Resource
		{
		public:
			DepthBuffer(ResourceView* view, s32 width, s32 height, TextureFormat format, const ClearStateDesc& clearStateDesc);

			// Return the width of the depth buffer
			s32 width() const;
			// Return the height of the depth buffer
			s32 height() const;
			// Return the format of the depth buffer
			TextureFormat format() const;
			// Return the resource view of the depth buffer
			ResourceView* resource_view();
			// Return the clear state of the depth buffer
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