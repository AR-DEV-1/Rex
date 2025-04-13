#pragma once

#include "rex_std/bonus/string.h"
#include "rex_std/chrono.h"
#include "rex_std/thread.h"
#include "rex_std/source_location.h"

#include "rex_engine/engine/types.h"
#include "rex_engine/engine/defines.h"

namespace rex
{
	class Timer
	{
	public:
		Timer(rsl::string_view name);

		rsl::string_view name() const;

		rsl::chrono::time_point<rsl::chrono::steady_clock> start_time() const;
		rsl::chrono::nanoseconds elapsed_time() const;
		void reset();

	private:
		rsl::string m_name;
		rsl::chrono::time_point<rsl::chrono::steady_clock> m_start_timepoint;
	};

}

