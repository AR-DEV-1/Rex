// NOLINTBEGIN

#include "rex_engine/engine/globals.h"
#include "rex_engine/engine/engine.h"
#include "rex_engine/cmdline/cmdline.h"

#define CATCH_CONFIG_RUNNER
#include "rex_unit_test/catch2/catch.hpp"

#define REX_CATCH_REGISTER_REPORTER(name, reporterType)                                                                                                                                                                                                \
        CATCH_INTERNAL_START_WARNINGS_SUPPRESSION                                                                                                                                                                                                        \
        CATCH_INTERNAL_SUPPRESS_GLOBALS_WARNINGS                                                                                                                                                                                                         \
        namespace                                                                                                                                                                                                                                        \
        {                                                                                                                                                                                                                                                \
          Catch::ReporterRegistrar<Catch:: reporterType> catch_internal_RegistrarFor##reporterType(name);                                                                                                                                                        \
        }                                                                                                                                                                                                                                                \
        CATCH_INTERNAL_STOP_WARNINGS_SUPPRESSION

REX_CATCH_REGISTER_REPORTER("console", ConsoleReporter)

class RexTestRunListener : public Catch::TestEventListenerBase {
public:
  RexTestRunListener(const Catch::ReporterConfig& _config)
    : Catch::TestEventListenerBase(_config)
  {
  }

  void testRunStarting(Catch::TestRunInfo const&) override
  {
  }

  void testCaseStarting(const Catch::TestCaseInfo& /*testInfo*/) override
  {
    // Some tests reset the commandlne
    if (rex::cmdline::instance() == nullptr)
    {
      rex::cmdline::init(rex::globals::make_unique<rex::CommandLine>(""));
    }

    // Some tests reset the engine globals
    if (rex::engine::instance() == nullptr)
    {
      s64 size = 1_kib;
      auto single_frame_allocator = rsl::make_unique<rex::TStackAllocator<rex::GlobalAllocator>>(size);
      auto scratch_allocator = rsl::make_unique<rex::TCircularAllocator<rex::GlobalAllocator>>(size);

      rex::engine::init(rex::globals::make_unique<rex::EngineGlobals>(rsl::move(scratch_allocator), rsl::move(single_frame_allocator)));
    }
  }

  void testRunEnded(const Catch::TestRunStats&) override
  {
    rex::engine::shutdown();
  }

};
CATCH_REGISTER_LISTENER(RexTestRunListener)

int main(int argc, char* argv[])
{
  return Catch::Session().run(argc, argv);
}

// NOLINTEND
