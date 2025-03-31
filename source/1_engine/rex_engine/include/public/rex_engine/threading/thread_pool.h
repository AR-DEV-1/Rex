#pragma once

#include "rex_engine/threading/thread.h"
#include "rex_engine/threading/thread_handle.h"
#include "rex_engine/engine/globals.h"

#include "rex_std/functional.h"
#include "rex_std/mutex.h"
#include "rex_std/vector.h"

namespace rex
{
	class ThreadPool
	{
	public:
		ThreadPool();
		//~ThreadPool();

		// Query if we have any idle threads available
		// An idle thread is a thread that's not executing a job at the moment.
		bool has_idle_threads();

		// Check if we have an idle thread
		// if so return it.
		ThreadHandle acquire_idle_thread();

		// Return a thread back to the pool
		void return_thread(internal::Thread* thread);

		// Destroy all threads. Used at shutdown of the engine
		void destroy_threads();

	private:
		rsl::vector<rsl::unique_ptr<internal::Thread>> m_threads; // Holds and owns all the threads used by the thread pool
		rsl::vector<internal::Thread*> m_idle_threads;            // Holds but doesn't own all the idle threads
		rsl::mutex m_threads_access_mtx;                // Mutex that's used to access the thread pool
	};

	namespace thread_pool
	{
		void init(globals::GlobalUniquePtr<ThreadPool> threadPool);
		ThreadPool* instance();
		void shutdown();
	} // namespace threading
} // namespace rex
