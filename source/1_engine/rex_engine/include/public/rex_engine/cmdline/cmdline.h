#pragma once

#include "rex_engine/string/stringid.h"
#include "rex_std/bonus/string.h"

namespace rex
{
	struct CommandLineArgument
	{
	public:
		CommandLineArgument(rsl::string_view name, rsl::string_view desc, rsl::string_view module);

		StringID name_id;
		rsl::string_view name;
		rsl::string_view desc;
		rsl::string_view module; // the module (eg. RexEngine) this argument is defined for, very useful for debugging as well as for the help command
	};

  class CommandLine
  {
  public:
    struct ActiveArgument
    {
      StringID argument_id;
      rsl::string_view argument;
      rsl::string_view value;
    };

    CommandLine(rsl::string_view cmdLine);

    // Print all possible commandline args a user can specify
    void help();
    // This is used to print the entire commandline
    void print();

    // This is used to scan if a certain argument is specified
    rsl::optional<rsl::string_view> get_argument(rsl::string_view arg);

  private:
    void parse_cmd_line(rsl::string_view cmdLine);
    void add_argument(rsl::string_view arg);
    bool verify_args(const CommandLineArgument* args, count_t argCount);
    rsl::string_view find_next_full_argument(rsl::string_view cmdLine, count_t startPos);

  private:
    rsl::vector<ActiveArgument> m_arguments;
    rsl::string_view m_command_line;
  };

  namespace cmdline
  {
    void init(rsl::unique_ptr<CommandLine> cmdLine);
    CommandLine* instance();
    void shutdown();
  } // namespace cmdline
} // namespace rex