#include "rex_engine/threading/thread_pool.h"

#include "rex_engine/system/system_info.h"
#include "rex_engine/threading/thread_event.h"
#include "rex_std/atomic.h"
#include "rex_std/mutex.h"
#include "rex_std/thread.h"

namespace rex
{
	ThreadPool::ThreadPool()
	{
		// To maximize core usage, we query the amount of local processors
		// and create a thread for each one of them.
		const s32 num_threads_to_spawn = rex::sys_info::num_logical_processors();
		m_threads.reserve(num_threads_to_spawn);
		m_idle_threads.reserve(num_threads_to_spawn);

		for (s32 idx = 0; idx < num_threads_to_spawn; ++idx)
		{
			m_threads.push_back(rsl::make_unique<threading::internal::Thread>());
			m_idle_threads.push_back(m_threads.back().get());
		}
	}

	// Query if we have any idle threads available
	// An idle thread is a thread that's not executing a job at the moment.
	bool ThreadPool::has_idle_threads()
	{
		const rsl::unique_lock lock(m_threads_access_mtx);
		return !m_idle_threads.empty();
	}

	// Check if we have an idle thread
	// if so return it.
	threading::ThreadHandle ThreadPool::acquire_idle_thread()
	{
		if (has_idle_threads())
		{
			const rsl::unique_lock lock(m_threads_access_mtx);
			threading::internal::Thread* thread = m_idle_threads.back();
			m_idle_threads.pop_back();
			return threading::ThreadHandle(thread);
		}

		return threading::ThreadHandle();
	}

	// Return a thread back to the pool
	void ThreadPool::return_thread(threading::internal::Thread* thread)
	{
		const rsl::unique_lock lock(m_threads_access_mtx);
		m_idle_threads.push_back(thread);
	}

	// Destroy all threads. Used at shutdown of the engine
	void ThreadPool::destroy_threads()
	{
		const rsl::unique_lock lock(m_threads_access_mtx);
		m_idle_threads.clear();
		m_threads.clear();
	}

	namespace thread_pool
	{
		rsl::unique_ptr<ThreadPool> g_thread_pool;
		void init(rsl::unique_ptr<ThreadPool> threadPool)
		{
			g_thread_pool = rsl::move(threadPool);
		}
		ThreadPool* instance()
		{
			return g_thread_pool.get();
		}
		void shutdown()
		{
			g_thread_pool.reset();
		}
	} // namespace threading
} // namespace rex