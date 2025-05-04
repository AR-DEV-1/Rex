#include "rex_engine/filesystem/native_filesystem.h"

#include "rex_engine/filesystem/path.h"
#include "rex_engine/filesystem/file.h"
#include "rex_engine/filesystem/directory.h"

namespace rex
{
  NativeFileSystem::NativeFileSystem(rsl::string_view root)
    : VfsBase(root)
  {
  }

  // --------------------------------
  // CREATING
  // --------------------------------
  Error NativeFileSystem::create_file(rsl::string_view path)
  {
    path = path::remove_quotes(path);
    path = path::unsafe_abs_path(path);

    return file::create_abspath(path);
  }
  Error NativeFileSystem::create_dir(rsl::string_view path)
  {
    path = path::remove_quotes(path);
    path = path::unsafe_abs_path(path);

    return directory::create_abspath(path);
  }
  Error NativeFileSystem::create_dirs(rsl::string_view path)
  {
    path = path::remove_quotes(path);
    path = path::unsafe_abs_path(path);

    return directory::create_recursive_abspath(path);
  }

  // --------------------------------
  // DELETING
  // --------------------------------
  Error NativeFileSystem::delete_file(rsl::string_view path)
  {
    path = path::remove_quotes(path);
    path = path::unsafe_abs_path(path);

    return file::del_abspath(path);
  }
  Error NativeFileSystem::delete_dir(rsl::string_view path)
  {
    path = path::remove_quotes(path);
    path = path::unsafe_abs_path(path);

    return directory::del_abspath(path);
  }
  Error NativeFileSystem::delete_dir_recursive(rsl::string_view path)
  {
    path = path::remove_quotes(path);
    path = path::unsafe_abs_path(path);

    return directory::del_recursive_abspath(path);
  }

  // --------------------------------
  // READING
  // --------------------------------
  memory::Blob NativeFileSystem::read_file(rsl::string_view path)
  {
    path = path::remove_quotes(path);
    path = path::unsafe_abs_path(path);

    return file::read_file_abspath(path);
  }
  s32 NativeFileSystem::read_file(rsl::string_view path, rsl::byte* buffer, s32 size)
  {
    path = path::remove_quotes(path);
    path = path::unsafe_abs_path(path);

    return file::read_file_abspath(path, buffer, size);
  }

  // --------------------------------
  // WRITING
  // --------------------------------
  Error NativeFileSystem::write_to_file(rsl::string_view path, const void* data, card64 size, AppendToFile shouldAppend)
  {
    path = path::unsafe_abs_path(path);

    if (shouldAppend)
    {
      return rex::file::append_text_abspath(path, rsl::string_view((const char8*)data, narrow_cast<s32>(size)));
    }
    else
    {
      return rex::file::write_to_file_abspath(path, data, size);
    }
  }
  Error NativeFileSystem::write_to_file(rsl::string_view filepath, rsl::string_view text, AppendToFile shouldAppend)
  {
    return write_to_file(filepath, text.data(), text.length(), shouldAppend);
  }
  Error NativeFileSystem::write_to_file(rsl::string_view filepath, const memory::Blob& blob, AppendToFile shouldAppend)
  {
    return write_to_file(filepath, blob.data(), blob.size(), shouldAppend);
  }

  // --------------------------------
  // CONVERTING
  // --------------------------------
  scratch_string NativeFileSystem::abs_path(rsl::string_view path)
  {
    path = path::remove_quotes(path);

    if (path::is_absolute(path))
    {
      return scratch_string(path);
    }

    return path::join(root(), path);
  }

  // --------------------------------
  // QUERYING
  // --------------------------------
  bool NativeFileSystem::exists(rsl::string_view path)
  {
    path = path::remove_quotes(path);
    path = abs_path(path);

    return directory::exists_abspath(path) || file::exists_abspath(path);
  }

  rsl::vector<rsl::string> NativeFileSystem::list_entries(rsl::string_view path)
  {
    path = path::remove_quotes(path);
    path = path::unsafe_abs_path(path);

    return directory::list_entries(path);
  }
  rsl::vector<rsl::string> NativeFileSystem::list_dirs(rsl::string_view path)
  {
    path = path::remove_quotes(path);
    path = path::unsafe_abs_path(path);

    return directory::list_dirs(path);
  }
  rsl::vector<rsl::string> NativeFileSystem::list_files(rsl::string_view path)
  {
    path = path::remove_quotes(path);
    path = path::unsafe_abs_path(path);

    return directory::list_files(path);
  }

}