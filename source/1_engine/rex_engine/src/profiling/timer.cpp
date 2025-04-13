#include "rex_engine/profiling/timer.h"

#include "rex_engine/filesystem/path.h"
#include "rex_engine/filesystem/vfs.h"
#include "rex_engine/filesystem/file.h"
#include "rex_std/mutex.h"
#include "rex_std/memory.h"

#include "rex_engine/memory/memory_tracking.h"
#include "rex_engine/task_system/task_system.h"

#include "rex_engine/diagnostics/log.h"
#include "rex_engine/profiling/profiling_session.h"

namespace rex
{
	DEFINE_LOG_CATEGORY(LogProfiler);

	Timer::Timer(rsl::string_view name)
		: m_name(name)
	{
		reset();
	}

	rsl::chrono::time_point<rsl::chrono::steady_clock> Timer::start_time() const
	{
		return m_start_timepoint;
	}
	rsl::chrono::nanoseconds Timer::elapsed_time() const
	{
		auto end = rsl::chrono::steady_clock::now();
		auto high_res_start = floating_point_nano_seconds(m_start_timepoint.time_since_epoch());
		auto elapsed_time = rsl::chrono::time_point_cast<rsl::chrono::nanoseconds>(end).time_since_epoch() - rsl::chrono::time_point_cast<rsl::chrono::nanoseconds>(m_start_timepoint).time_since_epoch();

		return elapsed_time;
	}

	rsl::string_view Timer::name() const
	{
		return m_name;
	}

	void Timer::reset()
	{
		m_start_timepoint = rsl::chrono::steady_clock::now();
	}
}