#include "rex_engine/gfx/imgui/imgui_utils.h"
#include "rex_engine/gfx/imgui/imgui_scoped_style.h"
#include "rex_engine/gfx/imgui/imgui_scoped_color.h"
#include "rex_engine/gfx/imgui/imgui_colors.h"

// #TODO: Remaining cleanup of development/Pokemon -> main merge. ID: IMGUI UTILS

namespace rex
{
	namespace imgui
	{
		// multiply a color with a multiplier, clamped at 1.0f
		ImColor color_with_multiplied_value(const ImColor& color, f32 multiplier)
		{
			const ImVec4& colRaw = color.Value;
			f32 hue, sat, val;
			ImGui::ColorConvertRGBtoHSV(colRaw.x, colRaw.y, colRaw.z, hue, sat, val);
			return ImColor::HSV(hue, sat, std::min(val * multiplier, 1.0f));
		}

		// dock a window (src) to another window (dst)
		void dock_to_window(rsl::string_view src, rsl::string_view dst, ImGuiDir dir)
		{
			ImGuiWindow* src_window = ImGui::FindWindowByName(src.data());
			ImGuiWindow* dst_window = ImGui::FindWindowByName(dst.data());

			if (src_window != nullptr && dst_window != nullptr)
			{
				dock_to_window(src_window, dst_window, dir);
			}
		}
		// dock a window (src) to another window (dst)
		void dock_to_window(ImGuiWindow* src, ImGuiWindow* dst, ImGuiDir dir)
		{
			ImGuiContext* context = ImGui::GetCurrentContext();

			ImGui::DockContextQueueDock(
				context,
				dst,																		// target
				nullptr,																// target node
				src,																		// payload
				dir,																		// simply add a tab
				0.0f,																		// split ratio
				false																		// split outer
			);
		}

		// returns true if the current item is hovered
		bool is_item_hovered(f32 delayInSeconds)
		{
			return ImGui::IsItemHovered() && GImGui->HoveredIdTimer > delayInSeconds; /*HoveredIdNotActiveTimer*/
		}

		// apply a tooltip to an item if it's currently hovered
		void set_tooltip(std::string_view txt, f32 delayInSeconds, ImVec2 padding)
		{
			if (is_item_hovered(delayInSeconds))
			{
				ScopedStyle tooltipPadding(ImGuiStyleVar_WindowPadding, padding);
				ScopedColor textCol(ImGuiCol_Text, colors::text_brighter);
				ImGui::SetTooltip("%s", txt.data());
			}
		}

		// offset a rect by a given offset
		ImRect shift_rect(const ImRect& rect, ImVec2 offset)
		{
			ImRect result = rect;
			result.Min.x += offset.x;
			result.Min.y += offset.y;
			result.Max.x += offset.x;
			result.Max.y += offset.y;
			return result;
		}
		// shift the y cursor with a given distance
		void shift_cursor_y(f32 distance)
		{
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + distance);
		}
		// shift the cursor by a given offset
		void shift_cursor(ImVec2 offset)
		{
			const ImVec2 cursor = ImGui::GetCursorPos();
			ImGui::SetCursorPos(ImVec2(cursor.x + offset.x, cursor.y + offset.y));
		}

		// create an imgui image from a texture
		void image(const rex::gfx::Texture2D* tex)
		{
			ImVec2 texSize{};
			texSize.x = static_cast<f32>(tex->width());
			texSize.y = static_cast<f32>(tex->height());
			ImGui::Image((ImTextureID)tex, texSize);
		}
	}
}