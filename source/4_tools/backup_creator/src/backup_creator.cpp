#include "rex_engine/engine/entrypoint.h"
#include "rex_engine/diagnostics/log.h"
#include "rex_engine/filesystem/path.h"
#include "rex_engine/filesystem/directory.h"
#include "rex_engine/filesystem/file.h"
#include "rex_engine/text_processing/json.h"
#include "rex_std/bonus/algorithms.h"

#include "rex_engine/event_system/event_system.h"
#include "rex_engine/event_system/events/app/quit_app.h"
#include "rex_engine/profiling/timer.h"
#include "rex_engine/task_system/task_system.h"

DEFINE_LOG_CATEGORY(LogBackup);

namespace backup_creator
{
  struct BackupInfo
  {
    s64 num_items;
    rsl::memory_size size;
  };

  class BackupCreator
  {
  public:
    BackupCreator()
      : m_backup_info({})
    {}

    void init()
    {
      rex::json::json blacklist = rex::json::read_from_file("BackupCreator/blacklist.json");
      m_blacklisted_directories = blacklist["BlacklistedDirectories"];
      m_blacklisted_files = blacklist["BlacklistedFiles"];
      m_blacklisted_extensions = blacklist["BlacklistedExtensions"];
    }

    void run(rsl::string_view dir)
    {
      rex::Timer timer("backup timer");
      backup_directory(dir, dir);

      REX_INFO(LogBackup, "Backed up {} items", m_backup_info.num_items);
      REX_INFO(LogBackup, "Total size: {} GiB", m_backup_info.size.size_in_gib());
      REX_INFO(LogBackup, "Took {} seconds", timer.elapsed_seconds());
    }

  private:
    void backup_directory(rsl::string_view root, rsl::string_view dir)
    {
      // We do the recursive parsing ourselves so we can log what we parse
      rsl::vector<rsl::string> entries = rex::directory::list_entries(dir, rex::directory::Recursive::no);

      for (rsl::string_view entry : entries)
      {
        rex::scratch_string fullpath = rex::path::join(dir, entry);

        if (rex::file::exists(fullpath))
        {
          if (rsl::contains(m_blacklisted_files.cbegin(), m_blacklisted_files.cend(), entry))
          {
            REX_DEBUG(LogBackup, "Skipping {} as it is blacklisted", fullpath);
            continue;
          }
          if (rsl::contains_if(m_blacklisted_extensions.cbegin(), m_blacklisted_extensions.cend(),
            [&](rsl::string_view extension) { return rex::path::extension(entry) == extension; }))
          {
            REX_DEBUG(LogBackup, "Skipping {} as its extension is blacklisted", fullpath);
            continue;
          }

          REX_INFO(LogBackup, "Backing up: {}", fullpath);
          rex::scratch_string backuppath = rex::path::join("H:\\Backup_21_04_2025", rex::path::rel_path(fullpath, root));
          rex::directory::create_recursive(rex::path::parent_path(backuppath));
          rex::run_async([=]() { rex::file::copy(fullpath, backuppath, rex::file::OverwriteIfExist::yes); });
          ++m_backup_info.num_items;
          m_backup_info.size += rex::file::size(fullpath);
        }
        else if (rex::directory::exists(fullpath))
        {
          if (rsl::contains(m_blacklisted_directories.cbegin(), m_blacklisted_directories.cend(), entry))
          {
            REX_DEBUG(LogBackup, "Skipping {} as it is blacklisted", fullpath);
            continue;
          }

          backup_directory(root, rsl::string(fullpath));
        }
        else
        {
          REX_DEBUG(LogBackup, "Skipping {} as it's not a file nor a directory", fullpath);
        }
      }
    }

  private:
    rsl::vector<rsl::string> m_blacklisted_directories;
    rsl::vector<rsl::string> m_blacklisted_files;
    rsl::vector<rsl::string> m_blacklisted_extensions;
    BackupInfo m_backup_info;
  };
  BackupCreator g_backup_creator;

  bool initialize(const rex::ApplicationCreationParams& /*appCreationParams*/)
  {
    g_backup_creator.init();
    return true;
  }
  void update()
  {
    g_backup_creator.run("D:\\");

    rex::event_system::instance()->enqueue_event(rex::QuitApp("Finished backup"));
  }
  void shutdown() {}

} // namespace backup_creator

namespace rex
{
  ApplicationCreationParams app_entry(PlatformCreationParams& platformParams)
  {
    ApplicationCreationParams app_params(platformParams);

    app_params.engine_params.app_init_func     = backup_creator::initialize;
    app_params.engine_params.app_update_func   = backup_creator::update;
    app_params.engine_params.app_shutdown_func = backup_creator::shutdown;

    return app_params;
  }
} // namespace rex
