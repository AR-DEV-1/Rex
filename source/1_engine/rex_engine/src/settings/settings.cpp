#include "rex_engine/settings/settings.h"

#include "rex_engine/diagnostics/log.h"
#include "rex_engine/filesystem/file.h"
#include "rex_engine/filesystem/vfs.h"
#include "rex_engine/filesystem/path.h"
#include "rex_engine/text_processing/ini.h"
#include "rex_engine/text_processing/text_processing.h"
#include "rex_std/algorithm.h"
#include "rex_std/unordered_map.h"

namespace rex
{
  DEFINE_LOG_CATEGORY(LogSettings);

  // Check if a certain setting exists
  bool SettingsManager::has_setting(rsl::string_view name) const
  {
    return get_setting(name).has_value();
  }

  // gets a setting from the global map
  rsl::string_view SettingsManager::get_string(rsl::string_view name, rsl::string_view defaultVal) const
  {
    return get_setting(name).value_or(defaultVal);
  }

  // Get the value of a setting as an int
  s32 SettingsManager::get_int(rsl::string_view name, s32 defaultVal) const
  {
    if (has_setting(name))
    {
      return rsl::stoi(get_string(name)).value_or(defaultVal);
    }

    return defaultVal;
  }
  // Get the value of a setting as a float
  f32 SettingsManager::get_float(rsl::string_view name, f32 defaultVal) const
  {
    if (has_setting(name))
    {
      return rsl::stof(get_string(name)).value_or(defaultVal);
    }

    return defaultVal;
  }
  bool SettingsManager::get_bool(rsl::string_view name, bool defaultVal) const
  {
    if (has_setting(name))
    {
      return rsl::stob(get_string(name)).value_or(defaultVal);
    }

    return defaultVal;
  }

  // set s asetting in the global map
  void SettingsManager::set(rsl::string_view name, rsl::string_view val) 
  {
    m_all_settings[name].assign(val);
  }

  // Set a setting from an int. This supports adding new settings
  void SettingsManager::set(rsl::string_view name, s32 val) 
  {
    m_all_settings[name].assign(rsl::to_string(val));
  }
  // Set a setting from a float. This supports adding new settings
  void SettingsManager::set(rsl::string_view name, f32 val) 
  {
    m_all_settings[name].assign(rsl::to_string(val));
  }

  // Load a settings file or directory and adds all settings files found to the settings
  void SettingsManager::load(rsl::string_view path)
  {
    if (!rex::vfs::instance()->exists(path))
    {
      REX_WARN(LogSettings, "{} does not exist, therefore its settings cannot be loaded", quoted(path));
      return;
    }

    if (rex::vfs::instance()->is_directory(path))
    {
      load_directory(path);
    }
    else if (rex::vfs::instance()->is_file(path))
    {
      load_file(path);
    }
    else
    {
      REX_ASSERT("A path was provided that's neither a file nor a directory");
    }
  }

  // Load a directory containing settings file and add all of them to the settings
  void SettingsManager::load_directory(rsl::string_view path)
  {
    const rsl::vector<rsl::string> files = vfs::instance()->list_entries(path, Recursive::yes);

    scratch_string fullpath;
    for (const rsl::string_view file : files)
    {
      fullpath.clear();
      path::join_to(fullpath, path, file);

      if (!vfs::instance()->is_file(fullpath))
      {
        continue;
      }

      REX_DEBUG(LogSettings, "Loading settings file: {}", fullpath);
      load_file(fullpath);
    }
  }

  // Load a settings file and adds it settings to the settings
  // This behaves the same as if you can "set" multiple times
  // for each setting in the file
  void SettingsManager::load_file(rsl::string_view path)
  {
    // of course if the path doesn't exist, we exit early
    if (!file::exists(path))
    {
      REX_ERROR(LogSettings, "Cannot load settings, file doesn't exist. File: {}", path);
      return;
    }

    // LogSettings are just plain ini files
    // so we can use the ini processor here
    rex::ini::Ini ini_content = rex::ini::read_from_file(path);
    if (ini_content.is_discarded())
    {
      REX_ERROR(LogSettings, "Cannot read settings file as ini parsing failed: {}", rex::path::abs_path(path));
      REX_ERROR(LogSettings, ini_content.parse_error().error_msg());
    }

    // Loop over the processed settings and add them to the global map
    rsl::vector<ini::IniBlock<>> blocks = ini_content.all_blocks();

    for (const ini::IniBlock<>& block : ini_content.all_blocks())
    {
      for (const auto [key, value] : block.all_items())
      {
        add_new_settings(block.header(), key, value);
      }
    }
  }

  // unload all settings
  void SettingsManager::unload()
  {
    m_all_settings.clear();
  }

  scratch_string SettingsManager::to_hash_key(rsl::string_view header, rsl::string_view key) const
  {
    // If a setting comes from a header, we add the header to the final name
    // and seperate it by a '.'
    scratch_string full_setting_name;
    if (!header.empty())
    {
      full_setting_name.assign(rsl::format("{}.{}", header, key));
    }
    else
    {
      full_setting_name.assign(key);
    }

    // remove all the spaces from a setting's name before we add it to the list
    remove_spaces(full_setting_name);

    return full_setting_name;
  }

  // add a new setting to the internal map
  // setting headers and keys are not allows to have spaces
  void SettingsManager::add_new_settings(rsl::string_view header, rsl::string_view key, rsl::string_view val)
  {
    scratch_string full_setting_name = to_hash_key(header, key);
    m_all_settings[rsl::move(full_setting_name)] = rsl::string(val);
  }

  rsl::optional<rsl::string_view> SettingsManager::get_setting(rsl::string_view name) const
  {
    if (m_all_settings.contains(name))
    {
      return m_all_settings.at(name);
    }
    return rsl::nullopt;
  }

  namespace settings
  {
    globals::GlobalUniquePtr<SettingsManager> g_settings_manager;
    void init(globals::GlobalUniquePtr<SettingsManager> settingsManager)
    {
      g_settings_manager = rsl::move(settingsManager);
    }
    SettingsManager* instance()
    {
      return g_settings_manager.get();
    }
    void shutdown()
    {
      g_settings_manager.reset();
    }

  } // namespace settings
} // namespace rex