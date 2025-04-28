#include "rex_engine/gfx/core/gpu_description.h"

namespace rex
{
  //-------------------------------------------------------------------------
  rsl::ostream& operator<<(rsl::ostream& os, const GpuDescription& desc)
  {
    os << "\n";
    os << "Description: " << desc.name << "\n";
    os << "GpuVendor Name: " << desc.vendor_name << "\n";
    os << "GpuVendor: " << gpu_vendor_to_string(desc.vendor_id) << "\n";
    os << "Device ID: " << desc.device_id << "\n";
    os << "Dedicated Video Memory: " << desc.dedicated_video_memory.size_in_mb() << "MB"
       << "\n";
    os << "Dedicated System Memory: " << desc.dedicated_system_memory.size_in_mb() << "MB"
       << "\n";
    os << "Shared System Memory: " << desc.shared_system_memory.size_in_mb() << "MB"
       << "\n";

    return os;
  }

  //-------------------------------------------------------------------------
  GpuDescription::GpuDescription()
      : name("")
      , vendor_name("")
      , vendor_id(GpuVendor::Unknown)
      , device_id(0)
      , dedicated_video_memory(0)
      , dedicated_system_memory(0)
      , shared_system_memory(0)
  {
  }
} // namespace rex