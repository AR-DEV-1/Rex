#include "rex_unit_test/rex_catch2.h"

#include "rex_engine/engine/invalid_object.h"

TEST_CASE("TEST - Invalid object - Construction")
{
	int x = rex::invalid_obj<int>();

	REX_CHECK(rex::is_invalid(x) == true);
}