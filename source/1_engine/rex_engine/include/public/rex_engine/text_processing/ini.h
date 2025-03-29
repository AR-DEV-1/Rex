#pragma once

#include "rex_engine/engine/casting.h"
#include "rex_engine/diagnostics/error.h"
#include "rex_engine/memory/blob_view.h"
#include "rex_std/bonus/utility/key_value.h"
#include "rex_std/string_view.h"
#include "rex_std/vector.h"
#include "rex_std/unordered_map.h"
#include "rex_std/bonus/functional.h"
#include "rex_std/bonus/string.h"

namespace rex
{
  namespace ini
  {
		// This holds a header (with or without a name)
		// and its internal items which are just key - value pairs
    template <typename Allocator = rsl::allocator>
		class IniBlock
		{
		public:
      using internal_string = rsl::basic_string<char8, rsl::ichar_traits<char8>, Allocator>;
      using internal_hash_map = rsl::unordered_map<
        internal_string,                 // key
        internal_string,                 // value
        rsl::hash_lower<internal_string>,      // hash
        rsl::equal_to_case_insensitive<internal_string>,  // comparison
        Allocator>;

      IniBlock() = default;
			IniBlock(rsl::string_view header, const rsl::vector<rsl::key_value<rsl::string_view, rsl::string_view>, Allocator>& items);

			rsl::string_view get(rsl::string_view key, rsl::string_view def = "") const;
			const internal_hash_map& all_items() const;

      rsl::string_view header() const;

		private:
      rsl::string_view m_header;
      internal_hash_map m_items;
		};

    template <typename Allocator = rsl::allocator>
    struct HeaderWithItems
    {
      rsl::string_view header;
      rsl::vector<rsl::key_value<rsl::string_view, rsl::string_view>, Allocator> items;
    };

    template <typename Allocator = rsl::allocator>
    class TIni
    {
    public:
      using internal_string = rsl::basic_string<char8, rsl::char_traits<char8>, Allocator>;
      using internal_hash_map = rsl::unordered_map<
        internal_string,                 // key
        IniBlock<Allocator>,              // value
        rsl::hash_lower<internal_string>,      // hash
        rsl::equal_to_case_insensitive<internal_string>,  // comparison
        Allocator>;

      // Construct an ini object with ini content
      explicit TIni(rsl::vector<HeaderWithItems<Allocator>, Allocator>&& headersWithItems);
      //explicit TIni(internal_hash_map&& headerWithItems);
      explicit TIni(Error parseError);

      // Return the value of a key, possible within a header
      rsl::string_view get(rsl::string_view header, rsl::string_view key, rsl::string_view def = "") const;

      // Return if the content is discard aka, the content is invalid and a parse error occurred
      bool is_discarded() const;

      // Return the parse error
      Error parse_error() const;

      // Return all items within this ini object
      rsl::vector<IniBlock<Allocator>, Allocator> all_blocks() const;

    private:
      internal_hash_map m_headers_with_items;
      rex::Error m_parse_error;
    };

    using Ini = TIni<rsl::allocator>;

    template <typename Allocator = rsl::allocator>
    TIni<Allocator> parse(rsl::string_view filepath);
    template <typename Allocator = rsl::allocator>
    TIni<Allocator> read_from_file(rsl::string_view filepath);
  }
} // namespace rex

#include "rex_engine/text_processing/ini.template.h"