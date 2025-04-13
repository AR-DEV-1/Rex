#include "rex_engine/threading/thread_handle.h"

#include "rex_engine/threading/thread_pool.h"
#include "rex_engine/diagnostics/assert.h"

namespace rex
{
	ThreadHandle::ThreadHandle()
		: m_thread(nullptr)
		, m_owning_thread_pool(nullptr)
	{}

	ThreadHandle::ThreadHandle(internal::Thread* thread, ThreadPool* owningPool)
		: m_thread(thread)
		, m_owning_thread_pool(owningPool)
	{
	}

	ThreadHandle::ThreadHandle(ThreadHandle&& other)
		: m_thread(rsl::exchange(other.m_thread, nullptr))
		, m_owning_thread_pool(rsl::exchange(other.m_owning_thread_pool, nullptr))
	{
	}

	ThreadHandle::~ThreadHandle()
	{
		return_me_to_thread_pool();
	}

	ThreadHandle& ThreadHandle::operator=(ThreadHandle&& other)
	{
		return_me_to_thread_pool();
		m_thread = rsl::exchange(other.m_thread, nullptr);
		m_owning_thread_pool = rsl::exchange(other.m_owning_thread_pool, nullptr);

		return *this;
	}

	void ThreadHandle::run(internal::thread_work_func&& func, void* arg)
	{
		m_thread->run(rsl::move(func), arg);
	}

	const internal::Thread* ThreadHandle::thread() const
	{
		return m_thread;
	}

	void ThreadHandle::return_me_to_thread_pool()
	{
		if (m_thread && m_owning_thread_pool)
		{
			m_owning_thread_pool->return_thread(m_thread);
		}
	}
} // namespace rex