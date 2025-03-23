// NOLINTBEGIN

#include "rex_engine/engine/mutable_globals.h"

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
    s64 size = 1_kib;

    rex::mut_globals().allocators.single_frame_allocator = rsl::make_unique<rex::StackAllocator<rex::GlobalAllocator>>(size);
    rex::mut_globals().allocators.scratch_allocator = rsl::make_unique<rex::CircularAllocator<rex::GlobalAllocator>>(size);
  }

  void testRunEnded(const Catch::TestRunStats&) override
  {
    rex::mut_globals().allocators.single_frame_allocator.reset();
    rex::mut_globals().allocators.scratch_allocator.reset();
  }

};
CATCH_REGISTER_LISTENER(RexTestRunListener)

int main(int argc, char* argv[])
{
  return Catch::Session().run(argc, argv);
}

// NOLINTEND
