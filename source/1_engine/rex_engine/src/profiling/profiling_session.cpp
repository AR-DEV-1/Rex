#include "rex_engine/profiling/profiling_session.h"

#include "rex_engine/filesystem/path.h"
#include "rex_engine/filesystem/file.h"

#include "rex_engine/memory/memory_types.h"

namespace rex
{
	ProfilingSession::ProfilingSession()
	{
		m_filepath.assign(rex::path::join(rex::engine::instance()->current_session_root(), "profile_result.json"));
		write_header();
	}

	ProfilingSession::~ProfilingSession()
	{
		write_footer();
	}

	void ProfilingSession::write_result(const ProfilingResult& result)
	{
		rsl::basic_stringstream<char8, rsl::char_traits<char8>, GlobalScratchAllocator> ss;
		scratch_string source_loc;
		scratch_string result_str;

		source_loc.assign(result.source_location.file_name());
		source_loc.replace("\\", "/");

		ss << ",{";
		ss << "\"cat\":\"function\",";
		ss << "\"dur\":" << (result.elapsed_time.count()) << ',';
		ss << "\"name\":\"" << result.name << "\",";
		ss << "\"ph\":\"X\",";
		ss << "\"pid\":0,";
		ss << "\"tid\":" << result.thread_id << ",";
		ss << "\"ts\":" << result.start_since_epoch.count() << ",";
		ss << "\"args\":";
		ss << "{";
		ss << "\"File\": \"" << source_loc << "\",";
		ss << "\"Line\": " << result.source_location.line();
		ss << "}";
		ss << "}\n";

		result_str = ss.str();

		file::append_line(m_filepath, result_str);
	}

	void ProfilingSession::write_header()
	{
		static rsl::string header;
		if (header.empty())
		{
			header += "{\n";
			header += "\"otherData\": {},\n";
			header += "\"displayTimeUnit\": \"ns\",\n"; // if the profile timing would change, make sure this is changed too
			header += "\"traceEvents\":[{}\n";
		}

		file::append_line(m_filepath, header);
	}
	void ProfilingSession::write_footer()
	{
		static rsl::string footer;
		if (footer.empty())
		{
			footer += "]}";
		}

		file::append_line(m_filepath, footer);
	}

	ProfilingTimer::ProfilingTimer(rsl::string_view name, rsl::source_location sourceLoc)
		: m_timer(name)
		, m_source_location(sourceLoc)
	{

	}
	ProfilingTimer::~ProfilingTimer()
	{
		auto elapsed_time = m_timer.elapsed_time();

		ProfilingResult result{
				rsl::big_stack_string(m_timer.name()),
				m_timer.start_time().time_since_epoch(),
				elapsed_time,
				rsl::this_thread::get_id(),
				m_source_location
		};

		profiling_session::instance()->write_result(result);

	}

	namespace profiling_session
	{
		globals::GlobalUniquePtr<ProfilingSession> g_profiling_session;
		void init(globals::GlobalUniquePtr<ProfilingSession> profilingSession)
		{
			g_profiling_session = rsl::move(profilingSession);
		}
		ProfilingSession* instance()
		{
			return g_profiling_session.get();
		}
		void shutdown()
		{
			g_profiling_session.reset();
		}
	}
}