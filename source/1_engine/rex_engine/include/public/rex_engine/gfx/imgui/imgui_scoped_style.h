#pragma once

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

namespace rex
{
	namespace imgui
	{
		class ScopedStyle
		{
		public:
			ScopedStyle(const ScopedStyle&) = delete;
			ScopedStyle& operator=(const ScopedStyle&) = delete;
			template<typename T>
			ScopedStyle(ImGuiStyleVar styleVar, T value) 
			{ 
				ImGui::PushStyleVar(styleVar, value); 
			}
			~ScopedStyle() 
			{ 
				ImGui::PopStyleVar(); 
			}
		};
	}
}