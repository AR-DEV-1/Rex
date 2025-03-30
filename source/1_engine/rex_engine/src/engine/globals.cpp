#include "rex_engine/engine/globals.h"

#include "rex_engine/diagnostics/assert.h"
#include "rex_engine/filesystem/path.h"
#include "rex_engine/system/process.h"

// #TODO: Remaining cleanup of development/Pokemon -> main merge. ID: REX GLOBALS

namespace rex
{
	// The engine globals get initialized twice,
	// once as a minimal setup, so we can boot up the engine
	// next time it gets fully initialized
	// To avoid requerying the paths on second initialization
	// we store them as a local static, as they don't change anyway
	EnginePaths init_engine_paths()
	{
		EnginePaths paths;

		scratch_string process_path = current_process::path();
		scratch_string data_root = path::find_in_parent("data", path::parent_path(process_path));

		paths.root.assign(path::parent_path(data_root));
		path::join_to(paths.engine_root, data_root, "rex");
		path::join_to(paths.project_root, data_root, engine::instance()->project_name());
		path::join_to(paths.sessions_root, data_root, "_sessions");
		path::join_to(paths.project_sessions_root, paths.sessions_root, engine::instance()->project_name());
		path::join_to(paths.current_session_root, paths.project_sessions_root, path::timepoint_for_filename(rsl::current_timepoint()));

		if (!directory::exists_abspath(paths.current_session_root))
		{
			directory::create(paths.current_session_root);
		}

		return paths;
	}
	EnginePaths& engine_paths()
	{
		static EnginePaths paths = init_engine_paths();
		return paths;
	}

	EngineGlobals::EngineGlobals(rsl::unique_ptr<ScratchAllocator> scratchAlloc, rsl::unique_ptr<SingleFrameAllocator> tempAlloc)
		: m_scratch_allocator(rsl::move(scratchAlloc))
		, m_single_frame_allocator(rsl::move(tempAlloc))
		, m_frame_info()
	{
		init_project_name();
	}

	void EngineGlobals::advance_frame()
	{
		m_frame_info.update();
		m_single_frame_allocator->reset();
	}

	void* EngineGlobals::scratch_alloc(s64 size)
	{
		return m_scratch_allocator->allocate(size);
	}
	void EngineGlobals::scratch_free(void* ptr)
	{
		return m_scratch_allocator->deallocate(ptr);
	}
	bool EngineGlobals::is_scratch_alloc(void* ptr) const
	{
		return m_scratch_allocator->has_allocated_ptr(ptr);
	}

	void* EngineGlobals::temp_alloc(s64 size)
	{
		return m_single_frame_allocator->allocate(size);
	}
	void EngineGlobals::temp_free(void* ptr)
	{
		return m_single_frame_allocator->deallocate(ptr);
	}
	bool EngineGlobals::is_temp_alloc(void* ptr) const
	{
		return m_single_frame_allocator->has_allocated_ptr(ptr);
	}

	const FrameInfo& EngineGlobals::frame_info() const
	{
		return m_frame_info;
	}

	// Returns the current project's name
	rsl::string_view EngineGlobals::project_name() const
	{
		return m_project_name;
	}

	// Returns the root of all files
	rsl::string_view EngineGlobals::root() const
	{
		return engine_paths().root;
	}

	// Returns the root directory of the engine files
	rsl::string_view EngineGlobals::engine_root() const
	{
		return engine_paths().engine_root;
	}

	// Returns the root directory of the current project
	rsl::string_view EngineGlobals::project_root() const
	{
		return engine_paths().project_root;
	}

	// Returns the root for all sessions data
	rsl::string_view EngineGlobals::sessions_root() const
	{
		return engine_paths().sessions_root;
	}

	// Returns the root for all sessions data of this project
	rsl::string_view EngineGlobals::project_sessions_root() const
	{
		return engine_paths().project_sessions_root;
	}

	// Returns the root for all files outputed during this session run (eg. logs)
	rsl::string_view EngineGlobals::current_session_root() const
	{
		return engine_paths().current_session_root;
	}

	void EngineGlobals::init_project_name()
	{
		// Get the project name from the exe
		path_stack_string process_path;
		current_process::path(process_path.data(), process_path.max_size());
		process_path.reset_null_termination_offset();
		rsl::string_view potential_project_name = path::stem(process_path);
		potential_project_name = potential_project_name.substr(0, potential_project_name.find('_')); // everything before the first underscore
		m_project_name = potential_project_name;
	}

	namespace engine
	{
		// the engine globals is a special case global
		// if it were a global variable, there's a chance 
		// the init func would gets called before the ctor of the global var
		// which would lead to unwanted results
		// therefore we have to store it as a local static
		rsl::unique_ptr<EngineGlobals>& engine_globals()
		{
			static rsl::unique_ptr<EngineGlobals> globals;
			return globals;
		}

		void init(rsl::unique_ptr<EngineGlobals> globals)
		{
			engine_globals() = rsl::move(globals);
		}
		EngineGlobals* instance()
		{
			return engine_globals().get();
		}
		void shutdown()
		{
			engine_globals().reset();
		}
	}
}
