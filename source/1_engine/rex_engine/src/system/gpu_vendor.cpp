#include "rex_engine/system/gpu_vendor.h"

namespace rex
{
	//-------------------------------------------------------------------------
	rsl::small_stack_string gpu_vendor_to_string(s32 vendor)
	{
		// Enum reflection is not possible here as the integer values are
		// outside the valid range of values [0, 127] for this enumeration type
		switch (static_cast<rex::GpuVendor>(vendor))
		{
		case rex::GpuVendor::Amd: return rsl::small_stack_string("AMD");
		case rex::GpuVendor::Nvidia: return rsl::small_stack_string("NVIDIA");
		case rex::GpuVendor::Intel: return rsl::small_stack_string("INTEL");
		default: return rsl::small_stack_string("Unknown GpuVendor");
		}
	}
	//-------------------------------------------------------------------------
	rsl::small_stack_string gpu_vendor_to_string(GpuVendor vendor)
	{
		return gpu_vendor_to_string(static_cast<s32>(vendor));
	}
}