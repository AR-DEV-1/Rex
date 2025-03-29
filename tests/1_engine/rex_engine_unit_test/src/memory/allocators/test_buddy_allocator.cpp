#include "rex_unit_test/rex_catch2.h"

#include "rex_engine/memory/allocators/buddy_allocator.h"

TEST_CASE("TEST - Buddy Allocator")
{
	rex::BuddyAllocator alloc(1_kib);

	s32* p1 = alloc.allocate<s32>();
	s32* p2 = alloc.allocate<s32>();
	s32* p3 = alloc.allocate<s32>();

	REX_CHECK(p1 != nullptr);
	REX_CHECK(p2 != nullptr);
	REX_CHECK(p3 != nullptr);

	*p1 = 1;
	*p2 = 2;
	*p3 = 3;

	alloc.deallocate(p1);
	alloc.deallocate(p2);
	alloc.deallocate(p3);
}
