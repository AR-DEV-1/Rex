#include "rex_engine/assets/map.h"

namespace rex
{
	Map::Map(MapDesc&& desc)
		: m_desc(rsl::move(desc))
	{}
}