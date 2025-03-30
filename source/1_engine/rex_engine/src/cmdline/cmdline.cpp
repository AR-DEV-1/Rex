#include "rex_engine/cmdline/cmdline.h"

#include "rex_engine/cmdline/cmd_args.h"
#include "rex_engine/diagnostics/assert.h"
#include "rex_engine/diagnostics/log.h"
#include "rex_engine/diagnostics/logging/log_macros.h"
#include "rex_engine/text_processing/text_processing.h"
#include "rex_std/bonus/hashtable.h"
#include "rex_std/bonus/types.h"
#include "rex_std/bonus/utility.h"
#include "rex_std/cstring.h"
#include "rex_std/format.h"
#include "rex_std/unordered_map.h"
#include "rex_std/vector.h"

namespace rex
{
  namespace internal
  {
    StringID create_string_id_for_arg(rsl::string_view arg)
    {
      return StringID::create(rsl::medium_stack_string(arg).to_lower());
    }
  } // namespace internal

	CommandLineArgument::CommandLineArgument(rsl::string_view name, rsl::string_view desc, rsl::string_view module)
		: name_id(internal::create_string_id_for_arg(name))
		, name(name)
		, desc(desc)
		, module(module)
	{
	}

  CommandLine::CommandLine(rsl::string_view cmdLine)
    : m_command_line(cmdLine)
  {
    // verify the auto generated command line arguments
    REX_ASSERT_X(verify_args(g_command_line_args.data(), g_command_line_args.size()), "You have ambiguous command line arguments");

    if (cmdLine.empty())
    {
      return;
    }

    parse_cmd_line(rsl::string_view(cmdLine));
  }

  void CommandLine::help()
  {
#ifndef REX_NO_LOGGING
    REX_INFO(LogEngine, "Listing Command line arguments");

    rsl::unordered_map<rsl::string_view, rsl::vector<CommandLineArgument>> project_to_arguments;

    for (const CommandLineArgument& cmd : g_command_line_args)
    {
      project_to_arguments[cmd.module].push_back(cmd);
    }

    for (auto& [project, cmds] : project_to_arguments)
    {
      REX_INFO(LogEngine, "");
      REX_INFO(LogEngine, "Commandline Arguments For {}", project);
      REX_INFO(LogEngine, "------------------------------");

      for (const CommandLineArgument& cmd : cmds)
      {
        REX_INFO(LogEngine, "\"{}\" - {}", cmd.name, cmd.desc);
      }
    }
#endif
  }

  void CommandLine::print()
  {
    if (m_command_line.length() > 0)
    {
      REX_INFO(LogEngine, "Commandline: {}", m_command_line);
    }
    else
    {
      REX_INFO(LogEngine, "No commandline given");
    }
  }

  rsl::optional<rsl::string_view> CommandLine::get_argument(rsl::string_view arg)
  {
    const StringID arg_id = internal::create_string_id_for_arg(arg);

    for (const ActiveArgument& active_arg : m_arguments)
    {
      if (arg_id == active_arg.argument_id)
      {
        return active_arg.value;
      }
    }

    return rsl::nullopt;
  }

  void CommandLine::parse_cmd_line(rsl::string_view cmdLine)
  {
    // skip all text that doesn't start until we find the first arg prefix
    const rsl::string_view arg_prefix = "-"; // all arguments should start with a '-'
    card32 start_pos = cmdLine.find(arg_prefix);
    if (start_pos == -1)
    {
      cmdLine = "";
    }
    else
    {
      cmdLine = cmdLine.substr(start_pos);
      start_pos = 0;
    }

    if (cmdLine.empty())
    {
      return;
    }

    // Here the actual command line parsing begins
    // It gets a bit tricky but here are some examples of command lines we all support
    // 1. -something -else
    // 2. -something=1 -else
    // 3. -something="path/to/something" -else
    // 4. -something="this has spaces" -else
    // So command line arguments are always split between a key and a possible value
    // Arguments always start with a '-', which is the start token of every key.
    // A key argument ends at the next space or '=' token, whichever comes first.
    // A value argument starts after the '=' sign, or the '"" that comes after the '=' token
    // a value arguments ends at the first space or at '"' if it started with one.

    while (start_pos != -1)
    {
      const rsl::string_view full_argument = find_next_full_argument(cmdLine, start_pos);
      const bool starts_with_prefix = full_argument.starts_with(arg_prefix);
      REX_ERROR_X(LogEngine, !starts_with_prefix, "argument '{}' doesn't start with '{}'. all arguments should start with '{}'", full_argument, arg_prefix, arg_prefix);
      if (starts_with_prefix) // NOLINT(readability-simplify-boolean-expr, readability-implicit-bool-conversion)
      {
        const rsl::string_view argument = full_argument.substr(arg_prefix.size());
        add_argument(argument);
      }

      start_pos = cmdLine.find_first_not_of(" \"", start_pos + full_argument.length()); // skip all additional spaces
    }

    if (start_pos != -1)
    {
      const rsl::string_view full_argument = find_next_full_argument(cmdLine, start_pos);
      const bool starts_with_prefix = full_argument.starts_with(arg_prefix);
      REX_ERROR_X(LogEngine, starts_with_prefix, "argument '{}' doesn't start with '{}'. all arguments should start with '{}'", full_argument, arg_prefix, arg_prefix);
      if (starts_with_prefix) // NOLINT(readability-simplify-boolean-expr, readability-implicit-bool-conversion)
      {
        const rsl::string_view argument = full_argument.substr(arg_prefix.size());
        add_argument(argument);
      }
    }
  }

  void CommandLine::add_argument(rsl::string_view arg)
  {
    const card32 equal_pos = arg.find('=');
    rsl::string_view key = "";
    rsl::string_view value = "";

    // if the argument is of type -MyArg=Something
    if (equal_pos != -1)
    {
      key = arg.substr(0, equal_pos);
      value = arg.substr(equal_pos + 1);

      // Remove the quote at the beginning if it starts with one
      value = remove_quotes(value);
    }
    // if the argument is of type -EnableSomething
    else
    {
      key = arg;
      value = "1"; // this is so we can easily convert to bool/int
    }

    const StringID key_id = internal::create_string_id_for_arg(key);
    auto cmd_it = rsl::find_if(g_command_line_args.cbegin(), g_command_line_args.cend(), [key_id](const CommandLineArgument& cmdArg) { return key_id == cmdArg.name_id; });

    if (cmd_it == g_command_line_args.cend())
    {
      REX_WARN(LogEngine, "Command '{}' passed in but it's not recognised as a valid command so will be ignored", key);
      return;
    }

    auto active_it = rsl::find_if(m_arguments.cbegin(), m_arguments.cend(), [key](const ActiveArgument& activeArg) { return rsl::stricmp(key.data(), activeArg.argument.data()) == 0; });

    if (active_it != m_arguments.cend())
    {
      REX_WARN(LogEngine, "Command '{}' was already passed in. passing the same argument multiple times is not supported. will be skipped", key);
      return;
    }

    m_arguments.push_back({ key_id, key, value });
  }

  bool CommandLine::verify_args(const CommandLineArgument* args, count_t argCount) // NOLINT(readability-convert-member-functions-to-static)
  {
    for (count_t i = 0; i < argCount; ++i)
    {
      const CommandLineArgument& lhs_arg = args[i];
      for (count_t j = 0; j < argCount; ++j)
      {
        if (i == j)
        {
          continue;
        }

        const CommandLineArgument& rhs_arg = args[j];
        if (lhs_arg.name_id == rhs_arg.name_id)
        {
          REX_ERROR(LogEngine, "This executable already has an argument for {} specified in 'g_command_line_args', please resolve the ambiguity by changing the code_generation file resulting in this ambiguity", lhs_arg.name);
          return false;
        }
      }
    }

    return true;
  }

  rsl::string_view CommandLine::find_next_full_argument(rsl::string_view cmdLine, count_t startPos) // NOLINT(readability-convert-member-functions-to-static)
  {
    const count_t space_pos = cmdLine.find_first_of(' ', startPos);
    count_t comma_pos = cmdLine.find_first_of('"', startPos);
    count_t pos_to_use = space_pos;
    if (comma_pos < space_pos && comma_pos != -1)
    {
      comma_pos = cmdLine.find_first_of('"', comma_pos + 1);
      pos_to_use = comma_pos;
    }

    return pos_to_use != -1 ? cmdLine.substr(startPos, pos_to_use - startPos) : cmdLine.substr(startPos);
  }

  namespace cmdline
  {
    rsl::unique_ptr<CommandLine> g_cmd_line;
    void init(rsl::unique_ptr<CommandLine> cmdLine)
    {
			g_cmd_line = rsl::move(cmdLine);
    }
    CommandLine* instance()
    {
      return g_cmd_line.get();
    }
    // we need to manually shut this down, otherwise it's memory might get tracked
    // by an already deleted memory tracking, causing an assert on shutdown
    void shutdown()
    {
      g_cmd_line.reset();
    }

  } // namespace cmdline
} // namespace rex