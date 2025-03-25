#pragma once

#include "rex_engine/engine/casting.h"
#include "rex_engine/diagnostics/error.h"
#include "rex_engine/memory/blob_view.h"
#include "rex_std/bonus/utility/key_value.h"
#include "rex_std/string_view.h"
#include "rex_std/vector.h"
#include "rex_std/unordered_map.h"

namespace rex
{
  template <typename StringType>
  struct hash_lower
  {
    constexpr rsl::hash_result operator()(const StringType& str) const
    {
      return rsl::crc32::compute_as_lower(str.data(), str.length());
    }
  };


  template <typename TypeToCompare = void>
  struct equal_to_lower
  {
    using is_transparent = rsl::true_type;

    constexpr bool operator()(const TypeToCompare& lhs, const TypeToCompare& rhs) const
    {
      if (lhs.length() != rhs.length())
      {
        return false;
      }

      return rsl::strincmp(lhs.data(), rhs.data(), lhs.length()) == 0;
    }

    /// RSL Comment: Different from ISO C++ Standard at time of writing (22/Aug/2022)
    // the standard doesn't template the second argument.
    // we do so we can, for example, compare a string with a const char*
    // without the second getting promoted to a string
    template <typename TypeToCompare2>
    constexpr bool operator()(const TypeToCompare& lhs, const TypeToCompare2& rhs) const
    {
      if (lhs.length() != rhs.length())
      {
        return false;
      }

      return rsl::strincmp(lhs.data(), rhs.data(), lhs.length()) == 0;
    }

    template <typename TypeToCompare2>
    constexpr bool operator()(const TypeToCompare2& lhs, const TypeToCompare& rhs) const
    {
      if (lhs.length() != rhs.length())
      {
        return false;
      }

      return rsl::strincmp(lhs.data(), rhs.data(), lhs.length()) == 0;
    }
  };


  namespace ini
  {
		// This holds a header (with or without a name)
		// and its internal items which are just key - value pairs
    template <typename Allocator = rsl::allocator>
		class IniBlock
		{
		public:
      using internal_string = rsl::basic_string<char8, rsl::char_traits<char8>, Allocator>;
      using internal_hash_map = rsl::unordered_map<
        internal_string,                 // key
        internal_string,                 // value
        hash_lower<internal_string>,      // hash
        equal_to_lower<internal_string>,  // comparison
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
        rsl::hash<internal_string>,      // hash
        rsl::equal_to<internal_string>,  // comparison
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