#include "rex_engine/text_processing/splitted_iterator.h"
#include "rex_engine/text_processing/text_processing.h"

namespace rex
{
	TextIterator::TextIterator()
		: m_text()
		, m_deliminators()
		, m_start(0)
		, m_end(m_text.npos())
		, m_sub_text_idx(-1)
	{}
	TextIterator::TextIterator(rsl::string_view path, rsl::string_view deliminators)
		: m_text(path)
		, m_deliminators(deliminators)
		, m_start(0)
		, m_end(path.find_first_of(m_deliminators))
		, m_sub_text_idx(0)
	{}

	TextIterator& TextIterator::operator++()
	{
		++m_sub_text_idx;

		if (m_end == m_text.npos())
		{
			*this = TextIterator();
		}
		else
		{
			m_start = m_text.find_first_not_of(m_deliminators, m_end);
			if (m_start == m_text.npos())
			{
				*this = TextIterator();
			}
		}

		m_end = m_text.find_first_of(m_deliminators, m_start + 1);
		return *this;
	}
	rsl::string_view TextIterator::operator*() const
	{
		s32 length = m_end != m_text.npos()
			? m_end - m_start
			: m_text.length() - m_start;

		return m_text.substr(m_start, length);
	}
	TextIterator& TextIterator::begin()
	{
		return *this;
	}
	TextIterator TextIterator::end()
	{
		return TextIterator();
	}
	bool TextIterator::operator==(const TextIterator& other)
	{
		return m_text == other.m_text;
	}
	bool TextIterator::operator!=(const TextIterator& other)
	{
		return !(*this == other);
	}
	s32 TextIterator::sub_text_index() const
	{
		return m_sub_text_idx;
	}

	LineIterator::LineIterator()
		: TextIterator()
	{}
	LineIterator::LineIterator(rsl::string_view text)
		: TextIterator(text, endline())
	{}
}