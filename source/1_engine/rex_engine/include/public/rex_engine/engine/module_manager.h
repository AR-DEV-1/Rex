#pragma once

#include "rex_engine/engine/module.h"
#include "rex_engine/engine/globals.h"

#include "rex_std/vector.h"

namespace rex
{
	// The module manager is a system that holds all our active modules
	// This allows the user to query at runtime what's compile in to the current process
	class ModuleManager
	{
	public:
		ModuleManager();

		// Return the current module
		// that is the module of the process itself
		// if you have a game called Bob, in a Bob visual studio project
		// this function will return the Bob module
		const Module* current() const;

	private:
		// Initialize a module based on its module file, who's path is passed in to this func
		Module* init_module(rsl::string_view modulePath);

		Module* m_current_module;
		rsl::vector<rsl::unique_ptr<Module>> m_all_modules;
	};

	namespace module_manager
	{
		void init(globals::GlobalUniquePtr<ModuleManager> moduleManager);
		ModuleManager* instance();
		void shutdown();
	}
}