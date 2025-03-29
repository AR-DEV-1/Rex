#include "rex_unit_test/rex_catch2.h"

#include "rex_engine/memory/global_allocators/global_debug_allocator.h"

#include "rex_unit_test/test_object.h"

TEST_CASE("TEST - Global Debug Allocator - Normal Allocation")
{
	rex::GlobalDebugAllocator alloc;

	s32 size = 10;
	void* ptr1 = alloc.allocate(size);
#if REX_DISABLE_DEBUG_ALLOCATION
	REX_CHECK(ptr1 == nullptr);
#else
	REX_CHECK(ptr1 != nullptr);
#endif
	alloc.deallocate(ptr1, size);

	using namespace rex::test;

	test_object::reset();

	REX_CHECK(test_object::num_created() == 0);
	REX_CHECK(test_object::num_ctor_calls() == 0);
	REX_CHECK(test_object::num_copy_ctor_calls() == 0);
	REX_CHECK(test_object::num_move_ctor_calls() == 0);
	REX_CHECK(test_object::num_dtor_calls() == 0);
	REX_CHECK(test_object::num_copy_assignment_calls() == 0);
	REX_CHECK(test_object::num_move_assignment_calls() == 0);

	test_object* ptr2 = alloc.allocate<test_object>();

#if REX_DISABLE_DEBUG_ALLOCATION
	REX_CHECK(ptr2 == nullptr);
	REX_CHECK(test_object::num_created() == 0);
	REX_CHECK(test_object::num_ctor_calls() == 0);
	REX_CHECK(test_object::num_copy_ctor_calls() == 0);
	REX_CHECK(test_object::num_move_ctor_calls() == 0);
	REX_CHECK(test_object::num_dtor_calls() == 0);
	REX_CHECK(test_object::num_copy_assignment_calls() == 0);
	REX_CHECK(test_object::num_move_assignment_calls() == 0);
#else
	alloc.construct(ptr2);

	REX_CHECK(ptr2 != nullptr);
	REX_CHECK(test_object::num_created() == 1);
	REX_CHECK(test_object::num_ctor_calls() == 1);
	REX_CHECK(test_object::num_copy_ctor_calls() == 0);
	REX_CHECK(test_object::num_move_ctor_calls() == 0);
	REX_CHECK(test_object::num_dtor_calls() == 0);
	REX_CHECK(test_object::num_copy_assignment_calls() == 0);
	REX_CHECK(test_object::num_move_assignment_calls() == 0);

	alloc.destroy(ptr2);
#endif

	alloc.deallocate(ptr2);

#if REX_DISABLE_DEBUG_ALLOCATION
	REX_CHECK(test_object::num_created() == 0);
	REX_CHECK(test_object::num_ctor_calls() == 0);
	REX_CHECK(test_object::num_copy_ctor_calls() == 0);
	REX_CHECK(test_object::num_move_ctor_calls() == 0);
	REX_CHECK(test_object::num_dtor_calls() == 0);
	REX_CHECK(test_object::num_copy_assignment_calls() == 0);
	REX_CHECK(test_object::num_move_assignment_calls() == 0);
#else
	REX_CHECK(test_object::num_created() == 1);
	REX_CHECK(test_object::num_ctor_calls() == 1);
	REX_CHECK(test_object::num_copy_ctor_calls() == 0);
	REX_CHECK(test_object::num_move_ctor_calls() == 0);
	REX_CHECK(test_object::num_dtor_calls() == 1);
	REX_CHECK(test_object::num_copy_assignment_calls() == 0);
	REX_CHECK(test_object::num_move_assignment_calls() == 0);
#endif
}