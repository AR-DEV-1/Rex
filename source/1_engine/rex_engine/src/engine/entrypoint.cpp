#include "rex_engine/engine/entrypoint.h"

#include "rex_engine/cmdline/cmdline.h"
#include "rex_engine/diagnostics/debug.h"
#include "rex_engine/diagnostics/log.h"
#include "rex_engine/engine/types.h"
#include "rex_engine/engine/engine.h"
#include "rex_engine/filesystem/vfs.h"
#include "rex_std/bonus/attributes.h"
#include "rex_std/internal/exception/exit.h"
#include "rex_std/thread.h"

namespace rex
{
  namespace internal
  {
    DEFINE_LOG_CATEGORY(LogPreInit);

    void log_pre_init_results()
    {
      REX_INFO(LogPreInit, "Startup: {}", rsl::current_timepoint());

      // Now log the commandline we started the app with
      cmdline::instance()->print();

      // Log early on if any sanitization is enabled
      // This is useful to have in the log file to make sure that correct sanitization is enabled when testing
      log_sanitization();

      REX_INFO(LogPreInit, "Project Name: {}", rex::engine::instance()->project_name());
      REX_INFO(LogPreInit, "Data Root: {}", rex::engine::instance()->data_root());
      REX_INFO(LogPreInit, "Session Directory: {}", rex::engine::instance()->current_session_root());
      REX_INFO(LogPreInit, "Log Path: {}", rex::project_log_path());
    }

    void pre_app_entry(REX_MAYBE_UNUSED const char8* cmdLine)
    {
      // Create minimal global allocators, in case we need them
      // they'll get destroyed and properly initialized later
      create_minimal_global_allocators();

      // Initialize the commandline first as this can influence everything else
      cmdline::init(globals::make_unique<CommandLine>(rsl::string_view(cmdLine)));

      // if we want to debug executables without a debugger, we need to break early
      // so we can attach a debugger and continue from then on
      // we'll have a timer in place to break for 2 minutes, if no debugger is attached
      // we close down the program
      if(cmdline::instance()->get_argument("BreakOnBoot"))
      {
        if(!wait_for_debugger())
        {
          rsl::exit(1); // exit if debugger didn't get attached
        }
      }

      // If the program was spawned without a debugger and we want to automatically attach one
      if(cmdline::instance()->get_argument("AttachOnBoot"))
      {
        attach_debugger();
      }

      // Now initialize all the logging diagnostics, including setting up file output
      // We need to do this here as we need the vfs to be initialized
      // We purposely don't initialize anything else here as this is meant to be a quick initialization phase
      // Proper initialization of other, more adavanced systems is deferred until the initialize function of the app
      log::init();

      // Now that the pre initialization has finished, including logging
      // log what we just did
      log_pre_init_results();
    }

    void post_app_shutdown()
    {
    }
  } // namespace internal
} // namespace rex