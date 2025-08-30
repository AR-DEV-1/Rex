#pragma once

#include "imgui/imgui_internal.h"

#include "rex_std/string_view.h"

#include "rex_engine/gfx/resources/texture_2d.h"
#include "rex_engine/gfx/resources/render_target.h"
#include "rex_engine/engine/types.h"

// #TODO: Remaining cleanup of development/Pokemon -> main merge. ID: IMGUI UTILS

namespace rex
{
	namespace imgui
	{
		// multiply a color with a multiplier, clamped at 1.0f
		ImColor color_with_multiplied_value(const ImColor& color, f32 multiplier);

		// dock a window (src) to another window (dst)
		void dock_to_window(rsl::string_view src, rsl::string_view dst, ImGuiDir dir);
		// dock a window (src) to another window (dst)
		void dock_to_window(ImGuiWindow* src, ImGuiWindow* dst, ImGuiDir dir);

		// returns true if the current item is hovered
		bool is_item_hovered(f32 delayInSeconds = 0.1f);
		// apply a tooltip to an item if it's currently hovered
		void set_tooltip(std::string_view text, f32 delayInSeconds = 0.1f, ImVec2 padding = ImVec2(5, 5));

		// offset a rect by a given offset
		ImRect shift_rect(const ImRect& rect, ImVec2 offset);
		// shift the y cursor with a given distance
		void shift_cursor_y(f32 distance);
		// shift the cursor by a given offset
		void shift_cursor(ImVec2 offset);
		
		// create an imgui image from a texture
		void image(const rex::gfx::Texture2D* tex);
		void image(const rex::gfx::RenderTarget* renderTarget);
	}
}