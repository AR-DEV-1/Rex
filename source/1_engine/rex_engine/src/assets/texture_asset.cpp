#include "rex_engine/assets/texture_asset.h"

#include "rex_engine/gfx/resources/texture_2d.h"

#include "rex_engine/gfx/graphics.h"
#include "rex_engine/gfx/resources/texture_2d.h"

namespace rex
{
	TextureAsset::TextureAsset(rsl::unique_array<u8> data, s32 width, s32 height, s32 numChannels)
	{
		m_texture_resource = gfx::gal::instance()->create_texture2d(width, height, gfx::TextureFormat::Unorm4, data.get());
	}
	TextureAsset::~TextureAsset() = default;

	const gfx::Texture2D* TextureAsset::texture_resource() const
	{
		return m_texture_resource.get();
	}

}