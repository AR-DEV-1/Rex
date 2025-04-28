#pragma once

namespace rex
{
	// aliasing platform namespace is a neat trick to deal with platform specific code
	// without having the user be aware of it, as they shouldn't care
	namespace win
	{

	}                    // namespace win

	using namespace win; // NOLINT(google-build-using-namespace)
}

// This is to avoid annoying compiler errors with min and max define
#ifndef NOMINMAX
#define NOMINMAX // prevent windows redefining min/max
#endif

#include <windows.h>
