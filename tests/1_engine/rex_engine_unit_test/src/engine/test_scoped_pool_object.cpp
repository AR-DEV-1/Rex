#include "rex_unit_test/rex_catch2.h"

#include "rex_engine/pooling/scoped_pooled_object.h"
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

TEST_CASE("TEST - Object With Destruction Callback - Construction from pool")
{
	rex::test::test_object::reset();

	rex::GrowingPool<rex::test::test_object> pool;
	rex::ScopedPoolObject<rex::test::test_object> scoped_object;
	{
		scoped_object = pool.request_scoped(
			[](const rsl::unique_ptr<rex::test::test_object>&) { return true; }, // find object
			[]() {return rsl::make_unique<rex::test::test_object>(); }); // create object

		*scoped_object = 1;

		REX_CHECK(rex::test::test_object::num_created() == 1);
		REX_CHECK(scoped_object.has_object() == true);
		REX_CHECK(scoped_object.get() != nullptr);
	}

	REX_CHECK(rex::test::test_object::num_created() == 1);
	REX_CHECK(rex::test::test_object::num_dtor_calls() == 0);
	REX_CHECK(scoped_object.has_object() == false);
	REX_CHECK(scoped_object.get() == nullptr);

	{
		scoped_object = pool.request_scoped(
			[](const rsl::unique_ptr<rex::test::test_object>&) { return true; }, // find object
			[]() {return rsl::make_unique<rex::test::test_object>(); }); // create object


		REX_CHECK(rex::test::test_object::num_created() == 1);
		REX_CHECK(rex::test::test_object::num_dtor_calls() == 0);
		REX_CHECK(scoped_object.has_object() == true);
		REX_CHECK(scoped_object.get() != nullptr);
		REX_CHECK(*scoped_object == 1);
	}

	REX_CHECK(rex::test::test_object::num_created() == 1);
	REX_CHECK(rex::test::test_object::num_dtor_calls() == 0);
	REX_CHECK(scoped_object.has_object() == false);
	REX_CHECK(scoped_object.get() == nullptr);
}