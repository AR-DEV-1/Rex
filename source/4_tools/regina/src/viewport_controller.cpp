#include "regina/viewport_controller.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "rex_engine/gfx/imgui/imgui_scoped_widget.h"
#include "rex_engine/gfx/imgui/imgui_utils.h"

namespace regina
{
	ViewportController::ViewportController(ViewportWidget* viewportWidget)
		: m_viewport_widget(viewportWidget)
	{
		// Initialize the render target early as this needs to be given to the render pass
		m_render_target = rex::gfx::gal::instance()->create_render_target(desc.width, desc.height, rex::gfx::TextureFormat::Unorm4Srgb);

		// Initialize a new render pass for the renderer, which will render the tiles of a map
		// TODO: Implement
		//rex::scratch_string tile_pass = rex::path::join(rex::engine::instance()->engine_root(), "renderpasses", "tilepass.json");
		//TileRenderPassDesc desc = rex::gfx::resource_cache::instance()->load_render_pass(tile_pass);

		rex::gfx::TilePassDesc renderpass_desc{};
		renderpass_desc.renderpass_desc.name = "Viewport Tile Render Pass";
		renderpass_desc.renderpass_desc.framebuffer_desc.emplace_back(m_render_target.get());

		// This data should be initialized through data, not hardcoded
		renderpass_desc.renderpass_desc.pso_desc.output_merger.raster_state = rex::gfx::gal::instance()->common_raster_state(rex::gfx::CommonRasterState::DefaultDepth);
		renderpass_desc.renderpass_desc.pso_desc.shader_pipeline.vs = rex::gfx::shader_lib::instance()->load(rex::path::join(rex::engine::instance()->project_root(), "shaders", "render_tile_vertex.hlsl"), rex::gfx::ShaderType::Vertex);
		renderpass_desc.renderpass_desc.pso_desc.shader_pipeline.ps = rex::gfx::shader_lib::instance()->load(rex::path::join(rex::engine::instance()->project_root(), "shaders", "render_tile_pixel.hlsl"), rex::gfx::ShaderType::Pixel);
		renderpass_desc.renderpass_desc.pso_desc.input_layout =
		{
			// Per vertex data
			rex::gfx::InputLayoutElementDesc{ rex::gfx::ShaderSemantic::Position, rex::gfx::ShaderArithmeticType::Float2 },
			rex::gfx::InputLayoutElementDesc{ rex::gfx::ShaderSemantic::TexCoord, rex::gfx::ShaderArithmeticType::Float2 },
		};

		renderpass_desc.tileset_desc = desc.tileset_desc;

		m_render_pass = rex::gfx::renderer::instance()->add_render_pass<rex::gfx::TilePass>(tilepass_desc);
	}

	void ViewportController::update()
	{
		// Send the render commands
		process_input();
		update_renderpass_data();
		draw_imgui();
	}

	void ViewportController::process_input()
	{
		// Process the input and update the camera accordingly

	}
	void ViewportController::update_renderpass_data()
	{
		m_render_pass->update_camera(m_camera);
		m_render_pass->reserve_objects(m_current_map->num_objects());
		for (const auto& obj : MapObjectsRange(m_current_map))
		{
			m_render_pass->push_object();
		}
	}
	void ViewportController::draw_imgui()
	{
		ImGuiWindowFlags window_flags{};
		window_flags |= ImGuiWindowFlags_NoTitleBar;

		if (auto widget = rex::imgui::ScopedWidget("Viewport", nullptr, window_flags))
		{
			// Make sure that we hide the tab bar, it doesn't make sense to have it for our viewport
			ImGui::GetCurrentWindow()->WindowClass.DockNodeFlagsOverrideSet |= ImGuiDockNodeFlags_HiddenTabBar;

			// Use the renderpass's render target as image for the viewport
			const rex::gfx::RenderTarget* render_target = m_render_pass->render_target();
			rex::imgui::image(render_target);
		}
	}
}