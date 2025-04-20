#pragma once

#include "rex_std/bonus/string.h"

#include "rex_engine/engine/types.h"

namespace rex
{
	//-------------------------------------------------------------------------
	// The VendorId is a unique identifier assigned by the PCI-SIG (Peripheral Component Interconnect Special Interest Group)
	// to identify the manufacturer of a PCI device, including graphics adapters.
	// The VendorId is a 16-bit unsigned integer that is typically included in
	// the PCI Configuration space of the device.
	enum class GpuVendor
	{
		Unknown = 0,
		Amd = 0x1002,
		Nvidia = 0x10DE,
		Intel = 0x163C
	};

	rsl::small_stack_string gpu_vendor_to_string(s32 vendor);
	rsl::small_stack_string gpu_vendor_to_string(GpuVendor vendor);
}