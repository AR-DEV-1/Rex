#include "rex_engine/filesystem/directory.h"

#include "rex_engine/filesystem/file.h"
#include "rex_engine/filesystem/path.h"

namespace rex
{
  namespace directory
  {
    // Get the size of all the files in the directory
    // possible going recursive over all the sub folders as well
    rsl::memory_size size(rsl::string_view path, Recursive goRecursive)
    {
      path = path::unsafe_abs_path(path);

      return size_abspath(path, goRecursive);
    }

    // Get the size of all the files in the directory
    // possible going recursive over all the sub folders as well
    rsl::memory_size size_abspath(rsl::string_view path, Recursive goRecursive)
    {
      REX_ASSERT_X(path::is_absolute(path), "argument is expected to be absolute here: {}", path);

      if (!exists_abspath(path))
      {
        return 0_bytes;
      }

      rsl::vector<rsl::string> entries = list_entries(path, goRecursive);
      rsl::memory_size dir_size = 0_bytes;
      for (const rsl::string_view entry : entries)
      {
        if (file::exists(entry))
        {
          dir_size += file::size(entry);
        }
      }

      return dir_size;
    }
  }
}
