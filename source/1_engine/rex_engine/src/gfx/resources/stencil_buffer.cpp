#include "rex_engine/gfx/resources/stencil_buffer.h"

namespace rex
{
	namespace gfx
	{
		StencilBuffer::StencilBuffer(ResourceView* view, s32 width, s32 height, TextureFormat format, u8 clearValue)
			: m_width(width)
			, m_height(height)
			, m_format(format)
			, m_view(view)
			, m_clear_value(clearValue)
		{

		}

		s32 StencilBuffer::width() const
		{
			return m_width;
		}
		s32 StencilBuffer::height() const
		{
			return m_height;
		}
		TextureFormat StencilBuffer::format() const
		{
			return m_format;
		}
		ResourceView* StencilBuffer::resource_view()
		{
			return m_view;
		}
		u8 StencilBuffer::clear_value() const
		{
			return m_clear_value;
		}

	}
}