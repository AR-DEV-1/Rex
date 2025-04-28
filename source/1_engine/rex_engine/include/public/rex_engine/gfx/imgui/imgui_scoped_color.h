#pragma once

#include "imgui/imgui.h"

namespace rex
{
	namespace imgui
	{
		class ScopedColor
		{
		public:
			ScopedColor(const ScopedColor&) = delete;
			ScopedColor(ScopedColor&&) = delete;
			ScopedColor& operator=(const ScopedColor&) = delete;
			ScopedColor& operator=(ScopedColor&&) = delete;
			
			template<typename T>
			ScopedColor(ImGuiCol colourId, T colour) 
			{
				ImGui::PushStyleColor(colourId, ImColor(colour).Value); 
			}
			~ScopedColor() 
			{
				ImGui::PopStyleColor(); 
			}
		};
	}
}