#include "rex_engine/engine/entrypoint.h"

#include "rex_engine/diagnostics/logging/log_macros.h"
#include "rex_engine/engine/types.h"
#include "rex_engine/profiling/timer.h"
#include "rex_engine/platform/win/crash_reporter/win_crash_handler.h"
#include "rex_std/bonus/utility.h"
#include "rex_std/iostream.h"
#include "rex_windows/app/console_application.h"
#include "rex_windows/app/gui_application.h"
#include "rex_windows/diagnostics/log.h"
#include "rex_windows/engine/platform_creation_params.h"

#define NOMINMAX
#include <Windows.h>
#include <processenv.h>

//-------------------------------------------------------------------------
int rex_win_entry(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPCSTR lpCmdLine, int nShowCmd)
{
  // This timer track how long early initialization takes
  // Early initialization is all initialization that takes place
  // before the app is spawned
  rex::Timer early_init_timer("Early init timer");

  // some systems need to be set up even before we get into the app entry function
  // commandline, logging and the vfs need to get set up before hand so the user has 
  // access to these system when they're setting up their app
  rex::internal::pre_app_entry(lpCmdLine);

  // The following log line needs to be here as the app result is logged using the same log category
  // If we log something now already (before the vfs gets uninitialized), the log path remains cached
  // If we wouldn't log anything before the vfs gets uninitialized, the program would crash as it can't decude the log path
  REX_INFO(LogWindows, "Starting Windows entrypoint");

  rex::PlatformCreationParams creation_params {};
  creation_params.instance      = hInstance;
  creation_params.prev_instance = hPrevInstance;
  creation_params.show_cmd      = nShowCmd;

  s32 result = 0;

  // We're wrapping executing in __try - __except block so that we can capture crashes
  // This allows us to perform particular logic when we encounter a crash.
  // Especially for when running tests, this can be very useful
  // See: https://learn.microsoft.com/en-us/cpp/cpp/try-except-statement?view=msvc-170
  __try
  {
    rex::ApplicationCreationParams app_params = rex::app_entry(creation_params);
    REX_INFO(LogWindows, "Early initializaion took {} ns", early_init_timer.elapsed_time().count());

    if(app_params.is_gui_app)
    {
      // this doesn't initialize anything but simply prepares the application for initialization
      rex::win::GuiApplication application(rsl::move(app_params));

      // this initializes, runs the loop and performs the shutdown
      result = application.run();
    }
    else
    {
      // this doesn't initialize anything but simply prepares the application for initialization
      rex::win::ConsoleApplication application(rsl::move(app_params));

      // this initializes, runs the loop and performs the shutdown
      result = application.run();
    }
  }
  __except(rex::win::report_crash_from_main_thread(GetExceptionInformation()), EXCEPTION_CONTINUE_SEARCH)
  {
    // Do nothing here as the code here doesn't get executed due to EXCEPTION_CONTINUE_SEARCH
    // handle crashing in the report_crash() function, then exit
    (void)0;
  }

  // by this point the application has finished and shutdown
  rex::internal::post_app_shutdown();

  return result;
}

//-------------------------------------------------------------------------
INT APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
  return rex_win_entry(hInstance, hPrevInstance, lpCmdLine, nShowCmd);
}

// main is always the entry point.
// on a graphical application however, we call into WinMain
// as this is supposed to be the entry point for a graphical application
// This is also the entry point that will be used without a console.
int main()
{
  STARTUPINFOW si;
  GetStartupInfoW(&si);

  s32 show_window = si.wShowWindow;

  if(!rsl::has_flag(si.dwFlags, STARTF_USESHOWWINDOW)) // this happens when running from the debugger
  {
    show_window = SW_SHOWNORMAL;
  }

  const int result = rex_win_entry(GetModuleHandle(nullptr), nullptr, GetCommandLineA(), show_window);

  return result;
}
