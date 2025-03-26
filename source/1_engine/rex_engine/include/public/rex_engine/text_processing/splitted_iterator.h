#pragma once

#include "rex_engine/engine/types.h"
#include "rex_std/string_view.h"

namespace rex
{
	class TextIterator
	{
	public:
		TextIterator();
		TextIterator(rsl::string_view text, rsl::string_view deliminators);

		TextIterator& operator++();
		rsl::string_view operator*() const;

		TextIterator& begin();
		TextIterator end();
		bool operator==(const TextIterator& other);
		bool operator!=(const TextIterator& other);

		s32 sub_text_index() const;

	private:
		rsl::string_view m_text;
		rsl::string_view m_deliminators;
		s32 m_start;
		s32 m_end;
		s32 m_sub_text_idx;
	};

	class LineIterator : public TextIterator
	{
	public:
		LineIterator();
		LineIterator(rsl::string_view text);
	};
}