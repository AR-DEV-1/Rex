#pragma once

#include "imgui/imgui.h"

namespace rex
{
	namespace imgui
	{
		class ScopedColourStack
		{
		public:
			ScopedColourStack(const ScopedColourStack&) = delete;
			ScopedColourStack& operator=(const ScopedColourStack&) = delete;

			template <typename ColourType, typename... OtherColours>
			ScopedColourStack(ImGuiCol firstColourID, ColourType firstColour, OtherColours&& ... otherColourPairs)
				: m_Count((sizeof... (otherColourPairs) / 2) + 1)
			{
				static_assert ((sizeof... (otherColourPairs) & 1u) == 0,
					"ScopedColourStack constructor expects a list of pairs of colour IDs and colours as its arguments");

				push_color(firstColourID, firstColour, std::forward<OtherColours>(otherColourPairs)...);
			}

			~ScopedColourStack() 
			{ 
				ImGui::PopStyleColor(m_Count); 
			}

		private:
			int m_Count;

			template <typename ColourType, typename... OtherColours>
			void push_color(ImGuiCol colourID, ColourType colour, OtherColours&& ... otherColourPairs)
			{
				if constexpr (sizeof... (otherColourPairs) == 0)
				{
					ImGui::PushStyleColor(colourID, ImColor(colour).Value);
				}
				else
				{
					ImGui::PushStyleColor(colourID, ImColor(colour).Value);
					push_color(std::forward<OtherColours>(otherColourPairs)...);
				}
			}
		};
	}
}