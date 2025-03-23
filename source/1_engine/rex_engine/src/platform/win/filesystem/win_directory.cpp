#include "rex_engine/platform/win/filesystem/win_directory.h"

#include "rex_engine/filesystem/file.h"
#include "rex_engine/filesystem/path.h"
#include "rex_engine/platform/win/diagnostics/win_call.h"
#include "rex_std/bonus/platform.h"
#include "rex_std/bonus/time/win/win_time_functions.h"
#include "rex_engine/containers/temp_vector.h"
#include "rex_std/bonus/utility/output_param.h"

namespace rex
{
  namespace directory
  {
    DEFINE_LOG_CATEGORY(LogDirectory);

    namespace internal
    {
      rsl::win::handle open_file_for_attribs(rsl::string_view path)
      {
        return rsl::win::handle(CreateFileA(path.data(), FILE_READ_ATTRIBUTES, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr));
      }

      void list_entries(rsl::string_view path, Recursive goRecursive, rsl::Out<rsl::vector<rsl::string>> outResult)
      {
        WIN32_FIND_DATAA ffd;
        scratch_string fullpath(path);
        if (!path::is_absolute(fullpath))
        {
          fullpath = rex::path::abs_path(path);
        }
        fullpath += "\\*";
        HANDLE find_handle = FindFirstFileA(fullpath.data(), &ffd);

        if (find_handle == INVALID_HANDLE_VALUE)
        {
          FindClose(find_handle);
          return;
        }

        temp_vector<rsl::string_view> dirs;
        scratch_string fullsubpath;
        do // NOLINT(cppcoreguidelines-avoid-do-while)
        {
          s32 length = rsl::strlen(ffd.cFileName);
          const rsl::string_view name(ffd.cFileName, length);
          if (name == "." || name == "..")
          {
            continue;
          }

          fullsubpath = path::join(fullpath, name);
          outResult.get().push_back(rsl::string(fullsubpath));
          if (goRecursive && directory::exists_abspath(fullsubpath))
          {
            dirs.push_back(outResult.get().back());
          }

        } while (FindNextFileA(find_handle, &ffd) != 0);

        // FindNextfile sets the error to ERROR_NO_MORE_FILES
        // if there are no more files found
        // We reset it here to avoid any confusion
        rex::win::clear_win_errors();

        for (rsl::string_view dir : dirs)
        {
          list_entries(dir, goRecursive, outResult);
        }

        return;
      }
    } // namespace internal

    // Create a new directory
    Error create(rsl::string_view path)
    {
      scratch_string fullpath;
      if (!path::is_absolute(path))
      {
        fullpath = path::abs_path(path);
        path = fullpath;
      }

      return create_abspath(path);
    }
    // Create a directory recursively, creating all sub directories until the leaf dir
    Error create_recursive(rsl::string_view path)
    {
      scratch_string fullpath;
      if (!path::is_absolute(path))
      {
        fullpath = path::abs_path(path);
        path = fullpath;
      }
      return create_recursive_abspath(path);
    }
    // Delete a directory
    Error del(rsl::string_view path)
    {
      scratch_string fullpath;
      if (!path::is_absolute(path))
      {
        fullpath = path::abs_path(path);
        path = fullpath;
      }
      return del_abspath(path);
    }
    // Delete a directory recursively, including all files and sub folders
    Error del_recursive(rsl::string_view path)
    {
      scratch_string fullpath;
      if (!path::is_absolute(path))
      {
        fullpath = path::abs_path(path);
        path = fullpath;
      }
      return del_recursive_abspath(path);
    }
    // Return if a directory exists
    bool exists(rsl::string_view path)
    {
      scratch_string fullpath;
      if (!path::is_absolute(path))
      {
        fullpath = path::abs_path(path);
        path = fullpath;
      }
      return exists_abspath(path);
    }
    // Copy a directory and its content
    Error copy(rsl::string_view src, rsl::string_view dst) // NOLINT(misc-no-recursion)
    {
      scratch_string full_src;
      if (!path::is_absolute(src))
      {
        full_src = path::abs_path(src);
        src = full_src;
      }
      scratch_string full_dst;
      if (!path::is_absolute(dst))
      {
        full_dst = path::abs_path(dst);
        dst = full_dst;
      }

      return copy_abspath(src, dst);
    }
    // Move/Rename a directory
    Error move(rsl::string_view src, rsl::string_view dst)
    {
      scratch_string full_src;
      if (!path::is_absolute(src))
      {
        full_src = path::abs_path(src);
        src = full_src;
      }
      scratch_string full_dst;
      if (!path::is_absolute(dst))
      {
        full_dst = path::abs_path(dst);
        dst = full_dst;
      }

      return move_abspath(src, dst);
    }

    // Returns true if the directory is empty, 
    // as in, it has no files or directories
    bool is_empty(rsl::string_view path)
    {
      // This doesn't have a "_abspath" overload as it doesn't make much sense
      // In fact, if it would, it'd be slower as it'd always have to create 2 strings
      // 1 for the fullpath in this func
      // 1 for the search query in the other func
      // at least by keeping everything in 1 func, there's a chance we have an optimization

      scratch_string fullpath(path);
      if (!path::is_absolute(fullpath))
      {
        fullpath = rex::path::abs_path(path);
      }
      WIN32_FIND_DATAA ffd;
      fullpath += "\\*";
      HANDLE find_handle = FindFirstFileA(fullpath.data(), &ffd);

      // If the path doesn't exist, we pretend that the directory is empty
      if (find_handle == INVALID_HANDLE_VALUE)
      {
        FindClose(find_handle);
        return true;
      }

      do // NOLINT(cppcoreguidelines-avoid-do-while)
      {
        s32 length = rsl::strlen(ffd.cFileName);
        const rsl::string_view name(ffd.cFileName, length);
        if (name == "." || name == "..")
        {
          continue;
        }

        return false;
      } while (FindNextFileA(find_handle, &ffd) != 0);

      // FindNextfile sets the error to ERROR_NO_MORE_FILES
      // if there are no more files found
      // We reset it here to avoid any confusion
      rex::win::clear_win_errors();

      return true;
    }

    // List the number of entries under a directory
    s32 num_entries(rsl::string_view path, Recursive goRecursive)
    {
      // This doesn't have a "_abspath" overload as it doesn't make much sense
      // In fact, if it would, it'd be slower as it'd always have to create 2 strings
      // 1 for the fullpath in this func
      // 1 for the search query in the other func
      // at least by keeping everything in 1 func, there's a chance we have an optimization

      WIN32_FIND_DATAA ffd;
      scratch_string fullpath(path);
      if (!path::is_absolute(fullpath))
      {
        fullpath = rex::path::abs_path(path);
      }
      fullpath += "\\*";
      HANDLE find_handle = FindFirstFileA(fullpath.data(), &ffd);

      if (find_handle == INVALID_HANDLE_VALUE)
      {
        FindClose(find_handle);
        return {};
      }

      s32 num_entries = 0;

      temp_vector<scratch_string> dirs;
      scratch_string fullsubpath;
      do // NOLINT(cppcoreguidelines-avoid-do-while)
      {
        s32 length = rsl::strlen(ffd.cFileName);
        const rsl::string_view name(ffd.cFileName, length);
        if (name == "." || name == "..")
        {
          continue;
        }

        fullsubpath = path::join(path, name);
        if (goRecursive && directory::exists_abspath(fullsubpath))
        {
          dirs.push_back(rsl::move(fullsubpath));
        }

        ++num_entries;

      } while (FindNextFileA(find_handle, &ffd) != 0);

      // FindNextfile sets the error to ERROR_NO_MORE_FILES
      // if there are no more files found
      // We reset it here to avoid any confusion
      rex::win::clear_win_errors();

      for (rsl::string_view dir : dirs)
      {
        num_entries += directory::num_entries(dir, goRecursive);
      }

      return num_entries;
    }
    // List the number of directories under a directory
    s32 num_dirs(rsl::string_view path)
    {
      // This doesn't have a "_abspath" overload as it doesn't make much sense
      // In fact, if it would, it'd be slower as it'd always have to create 2 strings
      // 1 for the fullpath in this func
      // 1 for the search query in the other func
      // at least by keeping everything in 1 func, there's a chance we have an optimization

      WIN32_FIND_DATAA ffd;
      scratch_string fullpath(path);
      if (!path::is_absolute(fullpath))
      {
        fullpath = rex::path::abs_path(path);
      }
      fullpath += "\\*";
      HANDLE find_handle = FindFirstFileA(fullpath.data(), &ffd);

      if (find_handle == INVALID_HANDLE_VALUE)
      {
        FindClose(find_handle);
        return {};
      }

      s32 num_dirs = 0;
      do // NOLINT(cppcoreguidelines-avoid-do-while)
      {
        s32 length = rsl::strlen(ffd.cFileName);
        const rsl::string_view name(ffd.cFileName, length);
        if (name == "." || name == "..")
        {
          continue;
        }

        const scratch_string full_filename = path::join(path, name);
        if (exists(full_filename))
        {
          ++num_dirs;
        }
      } while (FindNextFileA(find_handle, &ffd) != 0);

      // FindNextfile sets the error to ERROR_NO_MORE_FILES
      // if there are no more files found
      // We reset it here to avoid any confusion
      rex::win::clear_win_errors();

      return num_dirs;
    }
    // List the number of files under a directory
    s32 num_files(rsl::string_view path)
    {
      // This doesn't have a "_abspath" overload as it doesn't make much sense
      // In fact, if it would, it'd be slower as it'd always have to create 2 strings
      // 1 for the fullpath in this func
      // 1 for the search query in the other func
      // at least by keeping everything in 1 func, there's a chance we have an optimization

      WIN32_FIND_DATAA ffd{};
      scratch_string fullpath(path);
      if (!path::is_absolute(fullpath))
      {
        fullpath = rex::path::abs_path(path);
      }
      fullpath += "\\*";
      HANDLE find_handle = FindFirstFileA(fullpath.data(), &ffd);

      if (find_handle == INVALID_HANDLE_VALUE)
      {
        FindClose(find_handle);
        return {};
      }

      s32 num_files = 0;
      do // NOLINT(cppcoreguidelines-avoid-do-while)
      {
        const s32 length = rsl::strlen(ffd.cFileName);
        const rsl::string_view name(ffd.cFileName, length);
        scratch_string full_filename = path::join(path, name);
        if (file::exists(full_filename))
        {
          ++num_files;
        }
      } while (FindNextFileA(find_handle, &ffd) != 0);

      // FindNextfile sets the error to ERROR_NO_MORE_FILES
      // if there are no more files found
      // We reset it here to avoid any confusion
      rex::win::clear_win_errors();

      return num_files;
    }

    // List all entries under a directory
    rsl::vector<rsl::string> list_entries(rsl::string_view path, Recursive goRecursive)
    {
      rsl::vector<rsl::string> result;

      // We do this through a function taking in a output parameter
      // as it can massively improve performance due to less copying of strings
      // and less allocations as well
      internal::list_entries(path, goRecursive, rsl::Out(result));

      return result;
    }
    // List all directories under a directory
    rsl::vector<rsl::string> list_dirs(rsl::string_view path)
    {
      // This doesn't have a "_abspath" overload as it doesn't make much sense
      // In fact, if it would, it'd be slower as it'd always have to create 2 strings
      // 1 for the fullpath in this func
      // 1 for the search query in the other func
      // at least by keeping everything in 1 func, there's a chance we have an optimization

      WIN32_FIND_DATAA ffd;
      scratch_string fullpath(path);
      if (!path::is_absolute(fullpath))
      {
        fullpath = rex::path::abs_path(path);
      }
      fullpath += "\\*";
      HANDLE find_handle = FindFirstFileA(fullpath.data(), &ffd);

      if(find_handle == INVALID_HANDLE_VALUE)
      {
        FindClose(find_handle);
        return {};
      }

      rsl::vector<rsl::string> result;

      s32 num_dirs = directory::num_dirs(fullpath);
      result.reserve(num_dirs);

      do // NOLINT(cppcoreguidelines-avoid-do-while)
      {
        s32 length = rsl::strlen(ffd.cFileName);
        const rsl::string_view name(ffd.cFileName, length);
        if (name == "." || name == "..")
        {
          continue;
        }

        const scratch_string full_filename = path::join(path, name);
        if(exists(full_filename))
        {
          result.push_back(rsl::string(full_filename));
        }
      } while(FindNextFileA(find_handle, &ffd) != 0);

      // FindNextfile sets the error to ERROR_NO_MORE_FILES
      // if there are no more files found
      // We reset it here to avoid any confusion
      rex::win::clear_win_errors();

      return result;
    }
    // List all files under a directory
    rsl::vector<rsl::string> list_files(rsl::string_view path)
    {
      // This doesn't have a "_abspath" overload as it doesn't make much sense
      // In fact, if it would, it'd be slower as it'd always have to create 2 strings
      // 1 for the fullpath in this func
      // 1 for the search query in the other func
      // at least by keeping everything in 1 func, there's a chance we have an optimization

      WIN32_FIND_DATAA ffd {};
      scratch_string fullpath(path);
      if (!path::is_absolute(fullpath))
      {
        fullpath = rex::path::abs_path(path);
      }
      fullpath += "\\*";
      HANDLE find_handle = FindFirstFileA(fullpath.data(), &ffd);

      if(find_handle == INVALID_HANDLE_VALUE)
      {
        FindClose(find_handle);
        return {};
      }

      rsl::vector<rsl::string> result;

      s32 num_files = directory::num_files(fullpath);
      result.reserve(num_files);

      do // NOLINT(cppcoreguidelines-avoid-do-while)
      {
        const s32 length = rsl::strlen(ffd.cFileName);
        const rsl::string_view name(ffd.cFileName, length);
        scratch_string full_filename = path::join(path, name);
        if(file::exists(full_filename))
        {
          result.push_back(rsl::string(full_filename));
        }
      } while(FindNextFileA(find_handle, &ffd) != 0);

      // FindNextfile sets the error to ERROR_NO_MORE_FILES
      // if there are no more files found
      // We reset it here to avoid any confusion
      rex::win::clear_win_errors();

      return result;
    }

    // Return the creation time of a directory
    rsl::time_point creation_time(rsl::string_view path)
    {
      scratch_string fullpath;
      if (!path::is_absolute(path))
      {
        fullpath = path::abs_path(path);
        path = fullpath;
      }
      return creation_time_abspath(path);
    }
    // Return the access time of a directory
    rsl::time_point access_time(rsl::string_view path)
    {
      scratch_string fullpath;
      if (!path::is_absolute(path))
      {
        fullpath = path::abs_path(path);
        path = fullpath;
      }
      return access_time_abspath(path);
    }
    // Return the modification time of a directory
    rsl::time_point modification_time(rsl::string_view path)
    {
      scratch_string fullpath;
      if (!path::is_absolute(path))
      {
        fullpath = path::abs_path(path);
        path = fullpath;
      }
      return modification_time_abspath(path);
    }


    // ------------------------------------------------------------------------------
    //                          ABSOLUTE PATH IMPLEMENTATIONS
    // ------------------------------------------------------------------------------

    // Create a new directory
    Error create_abspath(rsl::string_view path)
    {
      REX_ASSERT_X(path::is_absolute(path), "argument is expected to be absolute here: {}", path);

      if (!rex::path::is_valid_path(path))
      {
        return Error::create_with_log(LogDirectory, "Cannot create directory \"{}\" as it's an invalid path", path);
      }

      if (exists_abspath(path))
      {
        return Error::create_with_log(LogDirectory, "Cannot create directory \"{}\" as it already exists", path);
      }

      const bool success = WIN_SUCCESS(CreateDirectoryA(path.data(), nullptr));

      return success
        ? Error::no_error()
        : Error::create_with_log(LogDirectory, "Failed to create directory at \"{}\"", path);
    }
    // Create a directory recursively, creating all sub directories until the leaf dir
    Error create_recursive_abspath(rsl::string_view path)
    {
      REX_ASSERT_X(path::is_absolute(path), "argument is expected to be absolute here: {}", path);

      const rsl::vector<rsl::string_view> splitted_paths = rsl::split(path, "/\\");

      rsl::string to_create;
      to_create.reserve(path.size());

      for (const rsl::string_view sub_path : splitted_paths)
      {
        to_create += sub_path;
        if (!exists_abspath(to_create))
        {
          const bool success = WIN_SUCCESS_IGNORE(CreateDirectoryA(to_create.data(), NULL), ERROR_ALREADY_EXISTS);

          if (!success)
          {
            return Error::create_with_log(LogDirectory, "Failed to create directory at \"{}\"", to_create);
          }
        }

        to_create += g_folder_seps;
      }

      return Error::no_error();
    }
    // Delete a directory, it's expected to be empty
    Error del_abspath(rsl::string_view path)
    {
      REX_ASSERT_X(path::is_absolute(path), "argument is expected to be absolute here: {}", path);

      if (!is_empty(path))
      {
        return Error::create_with_log(LogDirectory, "Failed to delete directory as it wasn't empty. Directory: \"{}\"", path);
      }

      const bool success = RemoveDirectoryA(path.data());

      return success
        ? Error::no_error()
        : Error::create_with_log(LogDirectory, "Failed to delete directory at \"{}\"", path);
    }
    // Delete a directory recursively, including all files and sub folders
    Error del_recursive_abspath(rsl::string_view path)
    {
      REX_ASSERT_X(path::is_absolute(path), "argument is expected to be absolute here: {}", path);

      Error error = Error::no_error();

      rsl::vector<rsl::string> dirs = list_dirs(path);
      for (rsl::string_view dir : dirs)
      {
        error = del_recursive_abspath(dir);
        if (error)
        {
          return Error::create_with_log(LogDirectory, "Failed to recursively delete \"{}\"", path);
        }
      }

      rsl::vector<rsl::string> files = list_files(path);
      for (rsl::string_view file : files)
      {
        error = file::del_abspath(file);
        if (error)
        {
          return Error::create_with_log(LogDirectory, "Failed to recursively delete \"{}\"", path);
        }
      }

      return directory::del_abspath(path);
    }
    // Return if a directory exists
    bool exists_abspath(rsl::string_view path)
    {
      REX_ASSERT_X(path::is_absolute(path), "argument is expected to be absolute here: {}", path);

      // It's possible the error returned here is ERROR_FILE_NOT_FOUND or ERROR_PATH_NOT_FOUND
      // because we can't ignore both, we just call it without wrapping it in WIN_CALL
      // and manually reset the windows error if an error has occurred
      const DWORD attribs = GetFileAttributesA(path.data());

      if (attribs == INVALID_FILE_ATTRIBUTES)
      {
        win::clear_win_errors();
        return false;
      }

      if ((attribs & FILE_ATTRIBUTE_DIRECTORY) != 0u)
      {
        return true;
      }

      return false;

    }
    // Copy a directory and its content
    Error copy_abspath(rsl::string_view src, rsl::string_view dst)
    {
      REX_ASSERT_X(path::is_absolute(src), "argument is expected to be absolute here: {}", src);
      REX_ASSERT_X(path::is_absolute(dst), "argument is expected to be absolute here: {}", dst);

      const rsl::vector<rsl::string> all_files = list_files(src);
      const rsl::vector<rsl::string> all_dirs = list_dirs(src);

      Error error = create_abspath(dst);
      if (error)
      {
        return error;
      }

      for (const rsl::string_view file_entry : all_files)
      {
        const scratch_string rel_path = path::rel_path_abspath(file_entry, src);
        const scratch_string entry_dst = path::join(dst, rel_path);
        error = file::copy_abspath(file_entry, entry_dst);
        if (error)
        {
          return error;
        }
      }

      for (const rsl::string_view dir_entry : all_dirs)
      {
        const scratch_string rel_path = path::rel_path_abspath(dir_entry, src);
        const scratch_string entry_dst = path::join(dst, rel_path);
        error = create_abspath(entry_dst);
        if (error)
        {
          return error;
        }
        error = copy_abspath(dir_entry, entry_dst);
        if (error)
        {
          return error;
        }
      }

      return Error::no_error();
    }
    // Move/Rename a directory
    Error move_abspath(rsl::string_view src, rsl::string_view dst)
    {
      REX_ASSERT_X(path::is_absolute(src), "argument is expected to be absolute here: {}", src);
      REX_ASSERT_X(path::is_absolute(dst), "argument is expected to be absolute here: {}", dst);

      const bool success = MoveFileA(src.data(), dst.data());

      return success
        ? Error::no_error()
        : Error::create_with_log(LogDirectory, "Failed to move directory from \"{}\" to \"{}\"", src, dst);
    }

    // Return the creation time of a directory
    rsl::time_point creation_time_abspath(rsl::string_view path)
    {
      REX_ASSERT_X(path::is_absolute(path), "argument is expected to be absolute here: {}", path);
      const rsl::win::handle file = internal::open_file_for_attribs(path);

      // When we have an invalid handle, we return 0
      if (!file.is_valid())
      {
        return rsl::time_point();
      }

      FILETIME creation_time{};
      GetFileTime(file.get(), &creation_time, nullptr, nullptr);
      const SYSTEMTIME sys_time = rsl::win::to_local_sys_time(creation_time);
      return rsl::timepoint_from_systime(sys_time);
    }
    // Return the access time of a directory
    rsl::time_point access_time_abspath(rsl::string_view path)
    {
      REX_ASSERT_X(path::is_absolute(path), "argument is expected to be absolute here: {}", path);
      const rsl::win::handle file = internal::open_file_for_attribs(path);

      // When we have an invalid handle, we return 0
      if (!file.is_valid())
      {
        return rsl::time_point();
      }

      FILETIME access_time{};
      GetFileTime(file.get(), nullptr, &access_time, nullptr);
      const SYSTEMTIME sys_time = rsl::win::to_local_sys_time(access_time);
      return rsl::timepoint_from_systime(sys_time);
    }
    // Return the modification time of a directory
    rsl::time_point modification_time_abspath(rsl::string_view path)
    {
      REX_ASSERT_X(path::is_absolute(path), "argument is expected to be absolute here: {}", path);
      const rsl::win::handle file = internal::open_file_for_attribs(path);

      // When we have an invalid handle, we return 0
      if (!file.is_valid())
      {
        return rsl::time_point();
      }

      FILETIME modification_time{};
      GetFileTime(file.get(), nullptr, nullptr, &modification_time);
      const SYSTEMTIME sys_time = rsl::win::to_local_sys_time(modification_time);
      return rsl::timepoint_from_systime(sys_time);
    }

  } // namespace directory
} // namespace rex