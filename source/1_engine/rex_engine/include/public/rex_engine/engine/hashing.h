#pragma once

#include "rex_std/functional.h"
#include "rex_std/type_traits.h"
#include "rex_std/bonus/functional.h"

// This provides a generic implementation of a hash for any POD type
// The hashing happens by converting the object to a byte array and hashing that

namespace rsl
{
  inline namespace v1
  {
    template<typename T>
    struct hash
    {
      static_assert(rsl::is_pod_v<T>, "Generic hasing can only be implemented for POD types");

      hash_result operator()(const T& object) const
      {
        return rsl::crc32::compute(&object, sizeof(object));
      }
    };
  }
}