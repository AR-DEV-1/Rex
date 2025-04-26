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

// Reduce the size of Win32 header files
// This also helps avoid some compiler errors
//#ifndef WIN32_LEAN_AND_MEAN
//#define WIN32_LEAN_AND_MEAN
//#endif

#include <windows.h>

// Undefining these as it can be a bit annoying and can cause compiler errors
// if you define a variable to be "near"
//#ifdef near
//#undef near
//#endif
//
//// Undefining these as it can be a bit annoying and can cause compiler errors
//// if you define a variable to be "far"
//#ifdef far
//#undef far
//#endif