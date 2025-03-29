#include "rex_unit_test/rex_catch2.h"

#include "rex_engine/timing/interval.h"

#include "rex_std/thread.h"

TEST_CASE("TEST - Interval - Timing")
{
  rex::Interval interval{};

  using namespace rsl::chrono_literals;
  rsl::this_thread::sleep_for(1ms);

  REX_CHECK(interval.milliseconds() > 0);
  REX_CHECK(interval.seconds() > 0);
}

TEST_CASE("TEST - Interval - Resetting")
{
  rex::Interval interval{};

  using namespace rsl::chrono_literals;
  rsl::this_thread::sleep_for(1s);

  interval.reset();
  REX_CHECK(interval.milliseconds() <= 1); // Because we're timing, it's possible the interval has increased by the time it gets here
  REX_CHECK(interval.seconds() <= 0.001f); // The interval might've increased even more by the time it got here
}