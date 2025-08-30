#pragma once

#include "rex_std/string_view.h"
#include "rex_std/string.h"

#include "rex_engine/text_processing/json.h"

namespace regina
{
	class Project
	{
	public:
		Project(rsl::string_view filepath);

		rsl::string_view name() const;

	private:
		rsl::string m_name;
	};

	namespace project_loader
	{
		rsl::unique_ptr<Project> create_new(rsl::string_view projectName);
		rsl::unique_ptr<Project> load_from_disk(rsl::string_view projectName);
	}
}