#pragma once

#include "rex_engine/engine/defines.h"
#include "rex_engine/engine/types.h"
#include "rex_engine/memory/global_allocators/global_debug_allocator.h"
#include "rex_engine/memory/memory_stats.h"
#include "rex_engine/memory/memory_tags.h"
#include "rex_engine/memory/memory_types.h"
#include "rex_engine/memory/allocators/untracked_allocator.h"
#include "rex_engine/diagnostics/stacktrace.h"
#include "rex_std/array.h"
#include "rex_std/bonus/attributes.h"
#include "rex_std/bonus/defines.h"
#include "rex_std/bonus/memory/memory_size.h"
#include "rex_std/bonus/types.h"
#include "rex_std/bonus/utility/enum_reflection.h"
#include "rex_std/bonus/utility/high_water_mark.h"
#include "rex_std/mutex.h"
#include "rex_std/vector.h"

namespace rex
{
  class MemoryHeader;
  struct MemoryAllocationStats;
  struct MemoryTrackingStats;

  // Memory stats with information about each tracked allocation 
  // Useful for debugging individual allocations after the fact and track down leaks
  struct MemoryTrackingStats
  {
    MemoryAllocationStats tracking_stats;

    // A list of all allocation headers. Good for memory allocation debugging
    debug_vector<MemoryHeader*> allocation_headers;
  };

  // An allocation callstack is a structure holding debug information of an allocation
  // This includes the callstack as well as the accumulation of the size of all allocations
  // that came from this callstack and then number of allocations
  class AllocationCallStack
  {
  public:
    AllocationCallStack(CallStack callstack, card64 size);

    void add_size(card64 size);
    void sub_size(card64 size);

    rsl::memory_size size() const;
    card32 alloc_count() const;

  private:
    CallStack m_callstack;
    rsl::memory_size m_size;
    card32 m_alloc_count;
  };

  // Holds information where the allocation came from
  // as well as where it could possibly get deleted from
  struct AllocationInfo
  {
    AllocationCallStack allocation_callstack;
    debug_vector<CallStack> deleter_callstacks;
  };

  class MemoryTracker
  {
  public:
    using UsagePerTag = rsl::array<rsl::high_water_mark<s64>, rsl::enum_refl::enum_count<MemoryTag>()>;

    MemoryTracker();
    MemoryTracker(const MemoryTracker&) = delete;
    MemoryTracker(MemoryTracker&&)      = delete;
    ~MemoryTracker();

    MemoryTracker& operator=(const MemoryTracker&) = delete;
    MemoryTracker& operator=(MemoryTracker&&)      = delete;

    void initialize(rsl::memory_size maxMemUsage);

    MemoryHeader* track_alloc(void* mem, card64 size);
    void track_dealloc(MemoryHeader* header);

    void push_tag(MemoryTag tag);
    void pop_tag();

    MemoryTag current_tag() const;

    void dump_stats_to_file(rsl::string_view filepath);

    REX_NO_DISCARD MemoryAllocationStats current_tracking_stats();
    REX_NO_DISCARD MemoryTrackingStats current_allocation_stats();      // deliberate copy as we don't want to have any race conditions when accessing
    REX_NO_DISCARD MemoryTrackingStats get_pre_init_stats(); // deliberate copy as we don't want to have any race conditions when accessing
    REX_NO_DISCARD MemoryTrackingStats get_init_stats();     // deliberate copy as we don't want to have any race conditions when accessing

    REX_NO_DISCARD MemoryTrackingStats get_stats_for_frame(card32 idx);

  private:
    // stores the headers for all allocations
    debug_vector<MemoryHeader*> m_allocation_headers;
    debug_hash_map<CallStack, AllocationInfo> m_allocation_info_table;
    rsl::high_water_mark<s64> m_mem_usage; // current memory usage
    s64 m_max_mem_budget;                  // maximum allowed memory usage
    MemoryStats m_mem_stats_on_startup;    // stats queried from the OS at init time
    rsl::mutex m_mem_tracking_mutex;
    UsagePerTag m_usage_per_tag;
    bool m_is_active;
    s32 m_num_total_allocations;
  };

  // an object that pushes a memory tag on construction
  // and pop it on destruction
  class MemoryTagScope
  {
  public:
    explicit MemoryTagScope(MemoryTag tag);
    MemoryTagScope(const MemoryTagScope&) = delete;
    MemoryTagScope(MemoryTagScope&&) = delete;
    ~MemoryTagScope();
    MemoryTagScope& operator=(const MemoryTagScope&) = delete;
    MemoryTagScope& operator=(MemoryTagScope&&) = delete;
  };

  // Memory tracker is the only thing that does not follow the global design pattern of Rex
  // this is because it needs to be initialized before anything else
  // as it's possible that a heap allocation happens before main
  MemoryTracker& mem_tracker();

} // namespace rex

#define REX_MEM_TAG_SCOPE(tag) const rex::MemoryTagScope REX_ANONYMOUS_VARIABLE(mem_tag_scope)(tag)