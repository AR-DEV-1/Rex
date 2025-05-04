#define STB_IMAGE_IMPLEMENTATION

#include "rex_engine/memory/global_allocators/global_allocator.h"

// Cannot implement the bellow until rex has a realloc implementation
namespace rex
{
	void* realloc(void* p, u64 size)
	{
		return GlobalAllocator().reallocate(p, size);
	}
}

#define STBI_MALLOC(sz)           operator new(sz)
#define STBI_REALLOC(p,newsz)     rex::realloc(p,newsz)
#define STBI_FREE(p)              operator delete(p)

#include "rex_engine/serialization/stb_image.h"