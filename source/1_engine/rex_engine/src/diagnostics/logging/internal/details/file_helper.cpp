#include "rex_engine/diagnostics/logging/internal/details/file_helper.h"

#include "rex_engine/diagnostics/assert.h"
#include "rex_engine/diagnostics/logging/internal/common.h"
#include "rex_engine/diagnostics/logging/internal/details/os.h"
#include "rex_engine/engine/types.h"
#include "rex_engine/filesystem/directory.h"
#include "rex_engine/filesystem/file.h"
#include "rex_engine/filesystem/filesystem_constants.h"
#include "rex_engine/filesystem/path.h"
#include "rex_engine/filesystem/vfs.h"
#include "rex_engine/memory/global_allocators/global_allocator.h"

#include <cerrno>

// IWYU pragma: no_include <built-in>

// NOLINTBEGIN(misc-definitions-in-headers)

namespace rex
{
  namespace log
  {
    namespace details
    {

      FileHelper::FileHelper(const FileEventHandlers& eventHandlers)
          : m_event_handlers(eventHandlers)
      {
      }

      FileHelper::~FileHelper()
      {
        close();
      }

      void FileHelper::open(rsl::string_view fname, bool truncate)
      {
        close();
        m_filename = filename_t(fname);

        // const auto* mode       = "ab";
        // const auto* trunc_mode = "wb";

        if(m_event_handlers.before_open)
        {
          m_event_handlers.before_open(m_filename);
        }
        for(int tries = 0; tries < s_open_tries; ++tries)
        {
          // create containing folder if not exists already.
          rex::directory::create(path::parent_path(fname));
          if(truncate && file::exists(fname))
          {
            // Truncate by opening-and-closing a tmp file in "wb" mode, always
            // opening the actual log-we-write-to in "ab" mode, since that
            // interacts more politely with eternal processes that might
            // rotate/truncate the file underneath us.
            file::trunc(fname);
          }
          // if(!os::fopen_s(&m_fd, fname, filename_t(mode)))
          //{
          //   if(m_event_handlers.after_open)
          //   {
          //     m_event_handlers.after_open(m_filename, m_fd);
          //   }
          //   return;
          // }

          details::os::sleep_for_millis(s_open_interval);
        }

        rex::debug_string err;
        err += "Failed opening file ";
        err += os::filename_to_str(m_filename);
        err += " for writing";
        err += " %d";
        printf("%s, %d", err.data(), errno);
      }

      void FileHelper::reopen(bool truncate)
      {
        if(m_filename.empty())
        {
          printf("Failed re opening file - was not opened before");
        }
        this->open(m_filename, truncate);
      }

      void FileHelper::flush()
      {
        if(fflush(m_fd) != 0)
        {
          rex::debug_string err;
          err += "Failed flush to file ";
          err += os::filename_to_str(m_filename);
          err += " %d";
          printf("%s, %d", err.data(), errno);
        }
      }

      void FileHelper::sync()
      {
        if(!os::fsync(m_fd))
        {
          rex::debug_string err;
          err += "Failed to fsync file ";
          err += os::filename_to_str(m_filename);
          err += " %d";
          printf("%s, %d", err.data(), errno);
        }
      }

      void FileHelper::close()
      {
        if(m_fd != nullptr)
        {
          if(m_event_handlers.before_close)
          {
            m_event_handlers.before_close(m_filename, m_fd);
          }

          REX_ASSERT_X(fclose(m_fd), "failed to close fd file");
          m_fd = nullptr;

          if(m_event_handlers.after_close)
          {
            m_event_handlers.after_close(m_filename);
          }
        }
      }

      void FileHelper::write(const memory_buf_t& buf)
      {
        const s32 msg_size = buf.size();
        const auto* data   = buf.data();

        vfs::write_to_file(m_filename, data, msg_size, vfs::AppendToFile::yes);

        // if(fwrite(data, 1, msg_size, m_fd) != msg_size)
        //{
        //   rex::debug_string err;
        //   err += "Failed writing to file ";
        //   err += os::filename_to_str(m_filename);
        //   err += " %d";
        //   printf("%s, %d", err.data(), errno);
        // }
      }

      size_t FileHelper::size() const
      {
        if(m_fd == nullptr)
        {
          rex::debug_string err;
          err += "Cannot use size() on closed file ";
          err += os::filename_to_str(m_filename);
          printf("%s", err.data());
        }
        return os::filesize(m_fd);
      }

      rsl::string_view FileHelper::filename() const
      {
        return m_filename;
      }

      //
      // return file path and its extension:
      //
      // "mylog.txt" => ("mylog", ".txt")
      // "mylog" => ("mylog", "")
      // "mylog." => ("mylog.", "")
      // "/dir1/dir2/mylog.txt" => ("/dir1/dir2/mylog", ".txt")
      //
      // the starting dot in filenames is ignored (hidden files):
      //
      // ".mylog" => (".mylog". "")
      // "my_folder/.mylog" => ("my_folder/.mylog", "")
      // "my_folder/.mylog.txt" => ("my_folder/.mylog", ".txt")

      FilenameWithExtension FileHelper::split_by_extension(rsl::string_view fname)
      {
        auto ext_index = fname.rfind('.');

        // no valid extension found - return whole path and empty string as
        // extension
        if(ext_index == filename_t::npos() || ext_index == 0 || ext_index == fname.size() - 1)
        {
          return FilenameWithExtension {fname, rsl::tiny_stack_string()};
        }

        // treat cases like "/etc/rc.d/somelogfile or "/abc/.hiddenfile"
        auto folder_index = fname.find_last_of(rex::g_folder_seps_filename);
        if(folder_index != filename_t::npos() && folder_index >= ext_index - 1)
        {
          return FilenameWithExtension {fname, rsl::tiny_stack_string()};
        }

        // finally - return a valid base and extension tuple
        return FilenameWithExtension {rsl::small_stack_string(fname.substr(0, ext_index)), rsl::tiny_stack_string(fname.substr(ext_index))};
      }

    } // namespace details
  }   // namespace log
} // namespace rex

// NOLINTEND(misc-definitions-in-headers)
