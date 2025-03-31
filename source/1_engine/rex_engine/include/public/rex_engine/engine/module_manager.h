#pragma once

#include "rex_engine/engine/module.h"
#include "rex_engine/engine/globals.h"

#include "rex_std/vector.h"

namespace rex
{
	class ModuleManager
	{
	public:
		ModuleManager();

		const Module* current() const;

	private:
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