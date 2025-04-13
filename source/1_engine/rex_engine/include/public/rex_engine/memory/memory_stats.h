#pragma once

#include "rex_engine/engine/types.h"
#include "rex_std/bonus/memory/memory_size.h"
#include "rex_std/bonus/utility/high_water_mark.h"
#include "rex_std/bonus/utility/enum_reflection.h"
#include "rex_engine/memory/memory_tags.h"
#include "rex_engine/memory/global_allocators/global_debug_allocator.h"
#include "rex_engine/memory/allocators/untracked_allocator.h"

namespace rex
{
  class MemoryHeader;

  // Memory stats about the system itself, independent of the process
  struct SystemMemoryStats
  {
    // total physical memory in the system
    rsl::memory_size total_physical_mem{};

    // available physical memory in the system (memory that isn't allocated yet)
    rsl::memory_size avail_physical_mem{};

    // total virtual memory in the system
    rsl::memory_size total_virtual_mem{};

    // total virtual memory in the system (memory that isn't allocated yet)
    rsl::memory_size avail_virtual_mem{};

    // the page size of the current system, aka how much memory memory is paged to disk
    rsl::memory_size page_size{};
  };

  // Memory stats coming from the operating system about the current process
  struct ProcessMemoryStats
  {
    // the amount of physical memory that's currently used by the process
    rsl::memory_size used_physical_mem{};

    // The peak physical memory usage of the process
    rsl::memory_size peak_physical_mem{};

    // the amount of virtual memory that's currently used by the process
    rsl::memory_size used_virtual_mem{};

    // The peak virtual memory usage of the process
    rsl::memory_size peak_virtual_mem{};

    // amount of page faults that have occurred
    // Note: this doesn't differentiate between hard and soft page faults
    // so don't assume that a high page fault count is bad
    card32 num_page_faults{};
  };

  // Memory stats coming from the engine's internal tracking system
  struct MemoryAllocationStats
  {
    // The total used memory
    rsl::memory_size used_memory;

    // The highest peak of memory usage
    rsl::memory_size max_used_memory;

    // The current number of allocations that are still allocated
    s32 num_alive_allocations;

    // The total number of allocations since tracking started
    s32 num_total_allocations;

    // The memory usage per memory tag
    using UsagePerTag = rsl::array<rsl::high_water_mark<s64>, rsl::enum_refl::enum_count<MemoryTag>()>;
    UsagePerTag usage_per_tag;
  };

  struct MemoryStats
  {
    MemoryAllocationStats mem_tracking_stats;
    SystemMemoryStats system_mem_stats;
    ProcessMemoryStats process_mem_stats;
  };

  MemoryAllocationStats query_mem_tracking_stats();
  SystemMemoryStats   query_system_mem_stats();
  ProcessMemoryStats  query_process_mem_stats();
  MemoryStats         query_all_memory_stats();

  void debug_log_mem_usage();
} // namespace rex

#ifdef REX_PLATFORM_WINDOWS
  #include "rex_engine/platform/win/memory/win_memory_stats.h"
#endif