#pragma once

#include "imgui/imgui.h"

namespace rex
{
	namespace imgui
	{
		class ScopedWidget
		{
		public:
			ScopedWidget(const char* name, bool* pOpen = nullptr, ImGuiWindowFlags flags = ImGuiWindowFlags_None);
			ScopedWidget(const ScopedWidget&) = delete;
			ScopedWidget(ScopedWidget&&) = delete;
			~ScopedWidget();

			ScopedWidget& operator=(const ScopedWidget&) = delete;
			ScopedWidget& operator=(ScopedWidget&&) = delete;

			explicit operator bool() const;

		private:
			bool m_is_open;
		};
	}
}