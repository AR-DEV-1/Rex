#pragma once

#include "rex_std/functional.h"
#include "rex_std/utility.h"

namespace rex
{
  template <typename PooledObject>
  class GrowingPool;

  // Wrapper around an object belonging to the pool
  // It'll automatically return it back to the pool when it goes out of scope
  template <typename T, typename PooledType = T>
  class ScopedPoolObject
  {
  public:
    ScopedPoolObject()
      : m_object(nullptr)
      , m_pool(nullptr)
    {}
    ScopedPoolObject(T* object, GrowingPool<PooledType>* pool)
      : m_object(object)
      , m_pool(pool)
    {}

    ScopedPoolObject(const ScopedPoolObject&) = delete;
    ScopedPoolObject(ScopedPoolObject&& other)
    {
      m_object = rsl::exchange(other.m_object, nullptr);
      m_pool = rsl::exchange(other.m_pool, nullptr);
    }

    ~ScopedPoolObject()
    {
      return_object_to_pool();
    }

    ScopedPoolObject& operator=(const ScopedPoolObject&) = delete;
    ScopedPoolObject& operator=(ScopedPoolObject&& other)
    {
      return_object_to_pool();
      m_object = rsl::exchange(other.m_object, nullptr);
      m_pool = rsl::exchange(other.m_pool, nullptr);

      return *this;
    }

    T* operator->()
    {
      return m_object;
    }
    const T* operator->() const
    {
      return m_object;
    }
    T& operator*()
    {
      return *m_object;
    }
    const T& operator*() const
    {
      return *m_object;
    }

    T* get()
    {
      return m_object;
    }
    const T* get() const
    {
      return m_object;
    }

    // Return if we still have an object.
    // We can lose an object after moving
    bool has_object() const
    {
      return m_object != nullptr;
    }

    // Return the object back to its pool
    void return_object_to_pool()
    {
      if (has_object())
      {
        m_pool->return_object(m_object);
        clear();
      }
    }

  protected:
    GrowingPool<T>* pool()
    {
      return m_pool;
    }
    void clear()
    {
      m_object = nullptr;
    }

  private:
    T* m_object;
    GrowingPool<PooledType>* m_pool;
  };

}