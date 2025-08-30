#pragma once

#include "rex_std/string_view.h"
#include "rex_std/string.h"

#include "rex_engine/assets/asset.h"

namespace regina
{
	class Scene : public rex::Asset
	{
	public:
		Scene(rsl::string_view name);

	private:
		rsl::string m_name;
	};
}