#include "rex_engine/gfx/resources/depth_buffer.h"

namespace rex
{
	namespace gfx
	{
		DepthBuffer::DepthBuffer(ResourceView* view, s32 width, s32 height, TextureFormat format, f32 clearValue)
			: m_width(width)
			, m_height(height)
			, m_format(format)
			, m_view(view)
			, m_clear_value(clearValue)
		{

		}

		s32 DepthBuffer::width() const
		{
			return m_width;
		}
		s32 DepthBuffer::height() const
		{
			return m_height;
		}
		TextureFormat DepthBuffer::format() const
		{
			return m_format;
		}
		ResourceView* DepthBuffer::resource_view()
		{
			return m_view;
		}
		f32 DepthBuffer::clear_value() const
		{
			return m_clear_value;
		}

	}
}