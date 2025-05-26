#pragma once

#include "regina/widgets/viewport_widget.h"
#include "rex_engine/gfx/rendering/render_passes/tile_pass.h"

namespace regina
{
	class ViewportController
	{
	public:
		ViewportController(ViewportWidget* viewportWidget);

		void update();

	private:
		void process_input();
		void update_renderpass_data();
		void draw_imgui();

	private:
		ViewportWidget* m_viewport_widget;
		rex::gfx::TilePass* m_render_pass;
	};
}