#pragma once

#include "rex_engine/engine/globals.h"
#include "rex_engine/memory/memory_types.h"
#include "rex_engine/platform/win/diagnostics/hr_call.h"
#include "rex_engine/platform/win/win_com_ptr.h"
#include "rex_std/bonus/string.h"
#include "rex_std/bonus/types.h"

#include <Windows.h>
#include <d3d12.h>
#include <stddef.h>

namespace rex::win
{
  class ComLibrary
  {
  public:
    // Initialize the library on first try
    ComLibrary();
    ~ComLibrary();

    ComLibrary(const ComLibrary&) = delete;
    ComLibrary(ComLibrary&&) = delete;

    ComLibrary& operator=(const ComLibrary&) = delete;
    ComLibrary& operator=(ComLibrary&&) = delete;

    // Read a symbolic link's path and return the path it actually points to
    scratch_string read_link(rsl::string_view filepath); // NOLINT(readability-convert-member-functions-to-static)

    // Create a com object using the com lib
    template <typename ComObject>
    wrl::ComPtr<ComObject> create_com_object()
    {
      wrl::ComPtr<ComObject> com_object;
      HR_CALL(CoCreateInstance(__uuidof(ComObject), NULL, CLSCTX_ALL, IID_PPV_ARGS(com_object.GetAddressOf())));
      return com_object;
    }

    // Create a com object using the com lib
    template <typename ComObject>
    wrl::ComPtr<ComObject> create_com_object(IID id)
    {
      wrl::ComPtr<ComObject> com_object;
      HR_CALL(CoCreateInstance(id, nullptr, CLSCTX_ALL, IID_PPV_ARGS(com_object.GetAddressOf())));
      return com_object;
    }
  };

  namespace com_lib
  {
    void init(globals::GlobalUniquePtr<ComLibrary> comLib);
    ComLibrary* instance();
    void shutdown();
    
  } // namespace com_lib
} // namespace rex::win
