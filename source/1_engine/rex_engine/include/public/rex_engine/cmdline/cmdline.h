#pragma once

#include "rex_engine/string/stringid.h"
#include "rex_engine/engine/globals.h"
#include "rex_std/bonus/string.h"
#include "rex_std/bonus/utility.h"

namespace rex
{
	struct CommandLineArgument
	{
	public:
		CommandLineArgument(rsl::string_view name, rsl::string_view desc, rsl::string_view filename);

		rsl::string_view name;
		rsl::string_view desc;
		rsl::string_view filename; // the filename from where the commandline arg was loaded from
	};

  class CommandLine
  {
  public:
    struct ActiveArgument
    {
      rsl::string_view argument;
      rsl::string_view value;
    };

    CommandLine(rsl::string_view cmdLine);

    void load_arguments_from_file(rsl::string_view file, rsl::string_view moduleName);

    // The post initialize step will reparse the commandline previously given
    // and validate them against the allowed arguments
    void post_init();

    // Print all possible commandline args a user can specify
    void help();
    // This is used to print the entire commandline
    void print();

    // This is used to scan if a certain argument is specified
    rsl::optional<rsl::string_view> get_argument(rsl::string_view arg);

  private:
    void load_hardcoded_arguments();
    DEFINE_YES_NO_ENUM(DisableInvalidArgumentWarning);
    void parse_cmd_line(rsl::string_view cmdLine, DisableInvalidArgumentWarning disableInvalidArgumentWarning);
    void add_argument(rsl::string_view arg, DisableInvalidArgumentWarning disableInvalidArgumentWarning);
    bool verify_arg(rsl::string_view argument, rsl::string_view filepath) const;
    rsl::string_view find_next_full_argument(rsl::string_view cmdLine, count_t startPos);

  private:
    rsl::vector<CommandLineArgument> m_allowed_arguments;

    rsl::vector<ActiveArgument> m_active_arguments;
    rsl::string_view m_command_line;
  };

  namespace cmdline
  {
    void init(globals::GlobalUniquePtr<CommandLine> cmdLine);
    CommandLine* instance();
    void shutdown();
  } // namespace cmdline
} // namespace rex