#pragma once

#include "rex_engine/engine/types.h"
#include "rex_engine/system/gpu_vendor.h"
#include "rex_std/bonus/memory/memory_size.h"
#include "rex_std/ostream.h"
#include "rex_std/string.h"

namespace rex
{
  struct GpuDescription
  {
    GpuDescription();

    // Human readable names of device and vendor
    rsl::small_stack_string name;           // The name of the GPU (eg. NVIDIA GeForce RTX 3060 Ti)
    rsl::small_stack_string vendor_name;    // The name of the vendor of the GPU (eg NVIDIA)

    // PCI IDs for your a device, this is what the OS uses
    GpuVendor vendor_id;                    // The PCI ID of the vendor (eg Nvidia)
    u32 device_id;                          // The PCI ID of the GPU itself. (eg 0x2489) You can look up the device in https://pci-ids.ucw.cz

    rsl::memory_size dedicated_video_memory;    // Num bytes of dedicated VRAM that's not shared with the CPU
    rsl::memory_size dedicated_system_memory;   // Num bytes of dedicated system memory that is not shared with the CPU
    rsl::memory_size shared_system_memory;      // Num bytes of shared system memory. This is the max number of system memory that can be used by the GPU
  };

  rsl::ostream& operator<<(rsl::ostream& os, const GpuDescription& desc);
} // namespace rex