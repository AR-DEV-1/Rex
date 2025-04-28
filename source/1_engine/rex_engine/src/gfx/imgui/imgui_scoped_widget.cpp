#include "rex_engine/gfx/imgui/imgui_scoped_widget.h"

namespace rex
{
	namespace imgui
	{
		ScopedWidget::ScopedWidget(const char* name, bool* pOpen, ImGuiWindowFlags flags)
		{
			m_is_open = ImGui::Begin(name, pOpen, flags);
		}
		ScopedWidget::~ScopedWidget()
		{
			ImGui::End();
		}
		ScopedWidget::operator bool() const
		{
			return m_is_open;
		}
	}
}