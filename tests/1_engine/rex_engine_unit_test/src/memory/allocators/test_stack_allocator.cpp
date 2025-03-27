#include "rex_unit_test/rex_catch2.h"

#include "rex_engine/memory/global_allocators/global_allocator.h"
#include "rex_engine/memory/allocators/stack_allocator.h"

#include "rex_engine/engine/types.h"

#include "rex_std/vector.h"

TEST_CASE("TEST - Stack Allocator - With Reset")
{
	rex::StackAllocator<rex::GlobalDebugAllocator> allocator(1_kib);

	s32* p1 = allocator.allocate<s32>();
	s32* p2 = allocator.allocate<s32>();
	s32* p3 = allocator.allocate<s32>();

	REX_CHECK(p1 != nullptr);
	REX_CHECK(p2 != nullptr);
	REX_CHECK(p3 != nullptr);
}

TEST_CASE("TEST - Stack Allocator - With Reset")
{
	rex::StackAllocator<rex::GlobalDebugAllocator> allocator(1_kib);
	s32* p1 = allocator.allocate<s32>();
	s32* p2 = allocator.allocate<s32>();
	s32* p3 = allocator.allocate<s32>();

	REX_CHECK(p1 != nullptr);
	REX_CHECK(p2 != nullptr);
	REX_CHECK(p3 != nullptr);

	allocator.reset();

	s32* p4 = allocator.allocate<s32>();
	s32* p5 = allocator.allocate<s32>();
	s32* p6 = allocator.allocate<s32>();

	REX_CHECK(p1 == p4);
	REX_CHECK(p2 == p5);
	REX_CHECK(p3 == p6);
}