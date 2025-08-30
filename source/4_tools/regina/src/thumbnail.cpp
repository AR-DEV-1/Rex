#include "regina/thumbnail.h"

namespace regina
{
	Thumbnail::Thumbnail(rex::TextureAsset* texture)
		: m_texture(rsl::move(texture))
	{}

	const rex::gfx::Texture2D* Thumbnail::texture() const
	{
		return m_texture->texture_resource();
	}

}