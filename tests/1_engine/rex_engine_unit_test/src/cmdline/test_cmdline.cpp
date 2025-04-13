#include "rex_unit_test/rex_catch2.h"

#include "rex_engine/cmdline/cmdline.h"

TEST_CASE("TEST - Commandline - empty commandline")
{
  rex::cmdline::init(rex::globals::make_unique<rex::CommandLine>(""));

  REX_CHECK(rex::cmdline::instance()->get_argument("something").has_value() == false);
}

TEST_CASE("TEST - Commandline - commandline with 1 argument")
{
  rex::cmdline::init(rex::globals::make_unique<rex::CommandLine>("-ExampleArgument"));

  REX_CHECK(rex::cmdline::instance()->get_argument("something").has_value() == false);
  REX_CHECK(rex::cmdline::instance()->get_argument("ExampleArgument").has_value() == true);
}

TEST_CASE("TEST - Commandline - commandline with 2 arguments")
{
  rex::cmdline::init(rex::globals::make_unique<rex::CommandLine>("-ExampleArgument -ExampleArgument2"));

  REX_CHECK(rex::cmdline::instance()->get_argument("something").has_value() == false);
  REX_CHECK(rex::cmdline::instance()->get_argument("ExampleArgument").has_value() == true);
  REX_CHECK(rex::cmdline::instance()->get_argument("ExampleArgument2").has_value() == true);
}

TEST_CASE("TEST - Commandline - commandline with equal sign")
{
  rex::cmdline::init(rex::globals::make_unique<rex::CommandLine>("-ExampleArgumentWithEqual=10"));

  REX_CHECK(rex::cmdline::instance()->get_argument("something").has_value() == false);
  REX_CHECK(rex::cmdline::instance()->get_argument("ExampleArgumentWithEqual").has_value() == true);
  REX_CHECK(rsl::stoi(rex::cmdline::instance()->get_argument("ExampleArgumentWithEqual").value()).value() == 10);
}

TEST_CASE("TEST - Commandline - commandline with equal sign and example argument")
{
  rex::cmdline::init(rex::globals::make_unique<rex::CommandLine>("-ExampleArgumentWithEqual=10 -ExampleArgument"));

  REX_CHECK(rex::cmdline::instance()->get_argument("something").has_value() == false);
  REX_CHECK(rex::cmdline::instance()->get_argument("ExampleArgument").has_value() == true);
  REX_CHECK(rex::cmdline::instance()->get_argument("ExampleArgumentWithEqual").has_value() == true);
  REX_CHECK(rsl::stoi(rex::cmdline::instance()->get_argument("ExampleArgumentWithEqual").value()).value() == 10);
}