#include "rex_unit_test/rex_catch2.h"

#include "rex_engine/memory/allocators/block_allocator.h"

TEST_CASE("TEST - Block Allocator - Matching Block Size")
{
	rex::BlockAllocator alloc(1_kib, sizeof(s32));

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

TEST_CASE("TEST - Block Allocator - Not Matching Block Size")
{
	rex::BlockAllocator alloc(1_kib, sizeof(s64));

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