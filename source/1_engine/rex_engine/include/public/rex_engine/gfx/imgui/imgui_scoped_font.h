#pragma once

#include "imgui/imgui.h"

namespace rex
{
	namespace imgui
	{
		class ScopedFont
		{
		public:
			ScopedFont(const ScopedFont&) = delete;
			ScopedFont& operator=(const ScopedFont&) = delete;
			ScopedFont(ImFont* font)
			{ 
				ImGui::PushFont(font); 
			}
			~ScopedFont()
			{ 
				ImGui::PopFont(); 
			}
		};
	}
}