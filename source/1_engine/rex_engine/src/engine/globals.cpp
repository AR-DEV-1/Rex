#include "rex_engine/engine/globals.h"

namespace rex
{
	namespace globals
	{
		bool s_enable_global_destruction = true;
		void enable_global_destruction()
		{
			s_enable_global_destruction = true;
		}
		void disable_global_destruction()
		{
			s_enable_global_destruction = false;
		}
		bool is_global_destruction_enabled()
		{
			return s_enable_global_destruction;
		}
	}
}
