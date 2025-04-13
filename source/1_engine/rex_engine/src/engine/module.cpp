#include "rex_engine/engine/module.h"

#include "rex_engine/filesystem/path.h"
#include "rex_engine/filesystem/file.h"
#include "rex_engine/system/process.h"
#include "rex_engine/diagnostics/log.h"

namespace rex
{
	DEFINE_LOG_CATEGORY(LogModule);

	Module::Module(rsl::string_view name, rsl::string_view dataPath, rsl::vector<Module*>&& dependencies)
		: m_name(name)
		, m_data_path(dataPath)
		, m_dependencies(rsl::move(dependencies))
	{
	}

	rsl::string_view Module::name() const
	{
		return m_name;
	}
	rsl::string_view Module::data_path() const
	{
		return m_data_path;
	}

	const rsl::vector<Module*>& Module::dependencies() const
	{
		return m_dependencies;
	}
}