#include "regina/widgets/viewport_widget.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

#include "rex_engine/gfx/graphics.h"
#include "rex_engine/gfx/imgui/imgui_scoped_widget.h"
#include "rex_engine/gfx/imgui/imgui_utils.h"

#include "rex_engine/engine/asset_db.h"

#include "rex_engine/filesystem/path.h"
#include "rex_std/bonus/math.h"

namespace regina
{
	ViewportWidget::ViewportWidget()
	{
		m_texture = rex::asset_db::instance()->load_from_binary<rex::TextureAsset>(rex::path::join("regina", "icons", "folder.png"));
	}

	bool ViewportWidget::on_update()
	{
		ImGuiWindowFlags window_flags{};
		window_flags |= ImGuiWindowFlags_NoTitleBar;

		if (auto widget = rex::imgui::ScopedWidget("Viewport", nullptr, window_flags))
		{
			// Make sure that we hide the tab bar, it doesn't make sense to have it for our viewport
			ImGui::GetCurrentWindow()->WindowClass.DockNodeFlagsOverrideSet |= ImGuiDockNodeFlags_HiddenTabBar;

			// The goal here is to render the scene within a viewport

			rex::imgui::image(m_texture->texture_resource());
		}

		return false;
	}

}