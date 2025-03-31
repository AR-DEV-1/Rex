#pragma once

#include "rex_engine/engine/globals.h"
#include "rex_engine/text_processing/json.h"

#include "rex_std/vector.h"
#include "rex_std/string_view.h"
#include "rex_std/string.h"

namespace rex
{
	class Module
	{
	public:
		Module(rsl::string_view name, rsl::string_view dataPath, rsl::vector<Module*>&& dependencies);
		const rsl::vector<Module*>& dependencies() const;

		rsl::string_view name() const;
		rsl::string_view data_path() const;

	private:
		rsl::string m_name;
		rsl::string m_data_path;
		rsl::vector<Module*> m_dependencies;
	};
}