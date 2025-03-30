#include "rex_engine/threading/thread_handle.h"

#include "rex_engine/threading/thread_pool.h"

namespace rex
{
  namespace threading
  {
    ThreadHandle::ThreadHandle()
      : m_thread(nullptr)
    {}

    ThreadHandle::ThreadHandle(internal::Thread* thread)
        : m_thread(thread)
    {
    }

    ThreadHandle::ThreadHandle(ThreadHandle&& other)
        : m_thread(rsl::exchange(other.m_thread, nullptr))
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
      if(m_thread)
      {
        thread_pool::instance()->return_thread(m_thread);
      }
    }
  } // namespace threading
} // namespace rex