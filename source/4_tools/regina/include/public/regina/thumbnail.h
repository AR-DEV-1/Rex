#pragma once

#include "rex_engine/assets/texture_asset.h"

#include "rex_std/memory.h"

namespace regina
{
	class Thumbnail
	{
	public:
		Thumbnail(rex::TextureAsset* texture);

		const rex::gfx::Texture2D* texture() const;

	private:
		rex::TextureAsset* m_texture;
	};
}