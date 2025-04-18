#include "rex_unit_test/rex_catch2.h"

#include "rex_engine/engine/scoped_pooled_object.h"
#include "rex_engine/pooling/growing_pool.h"
#include "rex_unit_test/test_object.h"

TEST_CASE("TEST - Object With Destruction Callback - Construction")
{
	rex::test::test_object::reset();

	rex::ScopedPoolObject<rex::test::test_object> scoped_object;

	REX_CHECK(rex::test::test_object::num_created() == 0);
	REX_CHECK(scoped_object.has_object() == false);
	REX_CHECK(scoped_object.get() == nullptr);
}

TEST_CASE("TEST - Object With Destruction Callback - Destruction")
{
	rex::test::test_object::reset();

	s32 x = 0;
	rex::test::test_object object(3);

	{
		rex::ScopedPoolObject<rex::test::test_object> scoped_object(&object, [&x](const rex::test::test_object* /*ptr*/) { ++x; });

		REX_CHECK(scoped_object->x() == 3);
		REX_CHECK(rex::test::test_object::num_created() == 1);
		REX_CHECK(x == 0);
	}

	REX_CHECK(rex::test::test_object::num_created() == 1);
	REX_CHECK(rex::test::test_object::num_dtor_calls() == 0);
	REX_CHECK(x == 1);
}