#pragma once

#include "rex_engine/engine/globals.h"

#include "rex_std/string.h"
#include "rex_std/unordered_map.h"
#include "rex_std/functional.h"

namespace rex
{
  class StringID;

  class StringPool
  {
  public:
    StringID find_or_store(rsl::string_view string);

  private:
    rsl::unordered_map<rsl::hash_result, rsl::string> m_entries;
  };

  namespace string_pool
  {
    void init(globals::GlobalUniquePtr<StringPool> stringPool);
    StringPool* instance();
    void shutdown();
  } // namespace string_pool
} // namespace rex