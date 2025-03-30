#include "rex_unit_test/rex_catch2.h"

#include "rex_engine/threading/thread_pool.h"
#include "rex_engine/engine/types.h"

#include "rex_engine/system/system_info.h"

TEST_CASE("TEST - Thread Pool - Construction")
{
	rex::ThreadPool thread_pool;

	REX_CHECK(thread_pool.has_idle_threads() == true);
}

TEST_CASE("TEST - Thread Pool - Thread Draining")
{
	rex::ThreadPool thread_pool;

	s32 num_logical_processors = rex::sys_info::num_logical_processors();
	rsl::vector<rex::threading::ThreadHandle> threads;
	while (num_logical_processors--)
	{
		threads.push_back(thread_pool.acquire_idle_thread());
	}

	REX_CHECK(thread_pool.has_idle_threads() == false);

	threads.clear();

	REX_CHECK(thread_pool.has_idle_threads() == true);
}

TEST_CASE("TEST - Thread Pool - Acquiring Idle Threads")
{
	rex::ThreadPool thread_pool;

	rex::threading::ThreadHandle thread = thread_pool.acquire_idle_thread();

	s32 x = 0;

	{
		// A thread always joins on destruction
		thread.run([&x](void*)
			{
				++x;
				return 0;
			});

		using namespace rsl::chrono_literals;
		rsl::this_thread::sleep_for(1s);
	}

	REX_CHECK(x == 1);
}