#pragma once

#include "regina/widgets/widget.h"

#include "rex_engine/assets/texture_asset.h"

namespace regina
{
	class ViewportWidget : public Widget
	{
	public:
		ViewportWidget();

	protected:
		bool on_update() override;

	private:
		rex::TextureAsset* m_texture;
	};
}