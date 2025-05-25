#include "regina/regina.h"

#include "regina/project.h"
#include "regina/content_manager.h"
#include "regina/scene_manager.h"
#include "regina/scene_serializer.h"
#include "regina/project_manager.h"
#include "regina/widgets/create_project_widget.h"
#include "regina/widgets/main_editor_widget.h"

#include "rex_engine/engine/asset_db.h"
#include "rex_engine/engine/engine.h"
#include "rex_engine/engine/globals.h"
#include "rex_engine/filesystem/vfs.h"
#include "rex_engine/filesystem/filesystem.h"
#include "rex_engine/event_system/events/app/quit_app.h"
#include "rex_engine/event_system/event_system.h"
#include "rex_engine/settings/settings.h"

DEFINE_LOG_CATEGORY(LogRegina);

#include "imgui/imgui.h"

namespace regina
{
	Regina::Regina(rsl::unique_ptr<Project> project)
		: m_project(rsl::move(project))
	{
		rex::vfs::instance()->mount(rex::MountingPoint::Editor, rex::path::join(rex::vfs::instance()->root(), "regina"));

		init_serializers();
		init_content_scope();
		init_settings();
		init_start_scene();

		init_scene_view();
		init_ui();
	}
	Regina::~Regina() = default;

	void Regina::update()
	{
		REX_ASSERT_X(m_active_widget, "No active widget in the editor");

		// Ideal desired code
		const Texture* last_frame = m_tile_renderer->last_rendered_frame();
		m_viewport->update(last_frame);

		if (m_active_widget->update())
		{
			m_active_widget->close();
		}
	}

	// Initialization
	void Regina::init_serializers()
	{
		rex::asset_db::instance()->add_serializer<Scene>(rsl::make_unique<SceneSerializer>());
	}
	void Regina::init_content_scope()
	{
		m_content_manager = rsl::make_unique<ContentManager>();
		m_content_manager->add_content(rex::engine::instance()->project_root());
	}
	void Regina::init_settings()
	{
		rex::scratch_string settings_path = rex::path::join(rex::vfs::instance()->mount_path(rex::MountingPoint::Editor), "settings");
		rex::settings::instance()->load_directory(settings_path);
	}
	void Regina::init_start_scene()
	{
		rex::scratch_string start_scene = rex::path::join(rex::engine::instance()->data_root(), rex::settings::instance()->get_string("StartScene"));
		if (!rex::vfs::instance()->is_file(start_scene))
		{
			REX_ERROR(LogRegina, "Start scene {} does not exist, loading default", rex::quoted(start_scene));
			start_scene.assign("default_map.json");
		}

		m_scene_manager = rsl::make_unique<SceneManager>();
		m_scene_manager->load_scene(start_scene);
	}
	void Regina::init_scene_view()
	{
	}
	void Regina::init_ui()
	{
		if (m_project)
		{
			spawn_main_widget();
		}
		else
		{
			spawn_create_project_widget();
		}
	}
	void Regina::spawn_main_widget()
	{
		REX_ASSERT_X(m_project, "Cannot spawn the main widget if we don't have a project. project is null");

		m_active_widget = rsl::make_unique<MainEditorWidget>(m_scene_manager.get());
	}

	// Project management
	void Regina::create_new_project(rsl::string_view projectName)
	{
		REX_INFO(LogRegina, "Creating new project with name \"{}\"", projectName);
		m_project = project_manager::create_new(projectName);
	}
	void Regina::spawn_create_project_widget()
	{
		m_active_widget = rsl::make_unique<CreateProjectWidget>(
			[this](rsl::string_view projectName)
			{
				create_new_project(projectName);
			}
		);

		m_active_widget->set_callback(WidgetEvent::Close, [this]() { spawn_main_widget(); });
	}
}