#pragma once

#include "rex_engine/engine/types.h"

#include "rex_std/utility.h"

namespace rex
{
	// A stack like container with a fixed size, defined at compile time
	template <typename T, s32 Size>
	class FixedStack
	{
	public:
		FixedStack()
			: m_data()
			, m_current_idx(0)
		{}

		void push(const T& item)
		{
			m_data[m_current_idx] = item;
			m_current_idx++;
		}
		void push(T&& item)
		{
			m_data[m_current_idx] = rsl::move(item);
			m_current_idx++;
		}
		void pop()
		{
			m_current_idx--;
		}

		const T& current() const
		{
			return m_data[m_current_idx];
		}
		T& current()
		{
			return m_data[m_current_idx];
		}

	private:
		rsl::array<T, Size> m_data;
		s32 m_current_idx;
	};
}