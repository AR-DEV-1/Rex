#include "rex_engine/memory/memory_tracking.h"

#include "rex_engine/app/core_application.h"
#include "rex_engine/diagnostics/assert.h"
#include "rex_engine/diagnostics/log.h"
#include "rex_engine/diagnostics/logging/log_macros.h"
#include "rex_engine/filesystem/vfs.h"
#include "rex_engine/frameinfo/frameinfo.h"
#include "rex_engine/memory/global_allocators/global_allocator.h"
#include "rex_engine/memory/memory_header.h"
#include "rex_engine/memory/memory_stats.h"
#include "rex_std/bonus/time/timepoint.h"
#include "rex_std/bonus/types.h"
#include "rex_engine/engine/engine.h"

namespace rex
{
#ifdef REX_MAX_ALLOWED_MEM_TAGS
  inline constexpr card32 g_max_allowed_mem_tags = REX_MAX_ALLOWED_MEM_TAGS;
#else
  inline constexpr card32 g_max_allowed_mem_tags = 100;
#endif

  rsl::array<MemoryTag, g_max_allowed_mem_tags>& thread_local_memory_tag_stack()
  {
    thread_local static rsl::array<MemoryTag, g_max_allowed_mem_tags> stack = {MemoryTag::Global};
    return stack;
  }

  card32& thread_local_mem_tag_index()
  {
    thread_local static card32 tag = 0;
    return tag;
  }

  MemoryTracker::MemoryTracker()
      : m_mem_usage(0)
      , m_max_mem_budget((rsl::numeric_limits<s64>::max)())
      , m_is_active(true)
  {
  }

  MemoryTracker::~MemoryTracker()
  {
    m_is_active = false;
  }

  void MemoryTracker::initialize(rsl::memory_size maxMemUsage)
  {
    // Remember that the OS has an overhead of memory allocation for the process itself
    // eg. Windows has a few MBs overhead for the process on startup, including page file usage and page faults
    // therefore a very low max memory usage could fall below this
    // That's why we track the initial memory usage before the initialization
    // so we can subtract this later, making sure that we only track the memory
    // that got allocated at runtime
    m_max_mem_budget       = static_cast<s64>(maxMemUsage);
    m_mem_stats_on_startup = query_all_memory_stats();
  }

  MemoryHeader* MemoryTracker::track_alloc(void* mem, card64 size)
  {
    const MemoryTag tag             = current_tag();
    const rsl::thread::id thread_id = rsl::this_thread::get_id();

    rex::MemoryHeader* dbg_header_addr   = static_cast<rex::MemoryHeader*>(GlobalDebugAllocator().allocate(sizeof(MemoryHeader)));
    const card32 frame_idx               = rex::engine::instance() 
      ? rex::engine::instance()->frame_info().index()
      : 0;
    const CallStack callstack            = current_callstack();

    // track the callstack, if we this callstack allocated memory before
    // add to the callstack the size of the memory we just allocated
    const rsl::unique_lock lock(m_mem_tracking_mutex);
    auto it          = m_allocation_info_table.find(callstack);
    if(it == m_allocation_info_table.end())
    {
      m_allocation_info_table.insert({callstack, AllocationInfo {AllocationCallStack(callstack, size)}});
    }
    else
    {
      it->value.allocation_callstack.add_size(size);
    }

    rex::MemoryHeader* header = new(dbg_header_addr) MemoryHeader(tag, mem, rsl::memory_size(size), thread_id, frame_idx, callstack);

    ++m_num_total_allocations;
    m_mem_usage += header->size().size_in_bytes();
    m_usage_per_tag[rsl::enum_refl::enum_integer(header->tag())] += header->size().size_in_bytes();
    m_allocation_headers.push_back(header);

    return header;
  }

  void MemoryTracker::track_dealloc(MemoryHeader* header)
  {
    // This is possible if static data gets deleted after the memory tracker is already destructed
    if(!m_is_active)
    {
      return;
    }

    // REX_WARN_X(LogEngine, header->frame_index() != globals::frame_info().index(), "Memory freed in the same frame it's allocated (please use single frame allocator for this)");

    // Postpone lock after logging to initialize the logger first ( on first access ).
    // Logger requires the same mutex to be locked and we cannot lock the same mutex twice from the same thread.
    const rsl::unique_lock lock(m_mem_tracking_mutex);

    m_mem_usage -= header->size().size_in_bytes();
    auto it = rsl::find(m_allocation_headers.cbegin(), m_allocation_headers.cend(), header);

    REX_ASSERT_X(it != m_allocation_headers.cend(), "Trying to remove a memory header that wasn't tracked");
    REX_ASSERT_X(m_mem_usage >= 0, "Mem usage below 0");

    auto alloc_info_it = m_allocation_info_table.find(header->callstack());
    REX_ASSERT_X(alloc_info_it != m_allocation_info_table.end(), "tracking a deallocation which allocation didn't get tracked");
    alloc_info_it->value.allocation_callstack.sub_size(header->size());

    // add unique deleter callstacks
    debug_vector<CallStack>& del_callstacks = m_allocation_info_table.at(header->callstack()).deleter_callstacks;
    const CallStack current_callstack      = rex::current_callstack();
    if(rsl::find(del_callstacks.cbegin(), del_callstacks.cend(), current_callstack) == del_callstacks.cend())
    {
      del_callstacks.push_back(rex::current_callstack());
    }

    if(alloc_info_it->value.allocation_callstack.size() == 0)
    {
      m_allocation_info_table.erase(header->callstack());
    }

    m_usage_per_tag[rsl::enum_refl::enum_integer(header->tag())] -= header->size().size_in_bytes();

    rex::GlobalDebugAllocator().deallocate(header, sizeof(MemoryHeader));
  }

  void MemoryTracker::push_tag(MemoryTag tag) // NOLINT(readability-convert-member-functions-to-static)
  {
    ++thread_local_mem_tag_index();
    thread_local_memory_tag_stack()[thread_local_mem_tag_index()] = tag;
  }
  void MemoryTracker::pop_tag() // NOLINT(readability-convert-member-functions-to-static)
  {
    --thread_local_mem_tag_index();
  }

  MemoryTag MemoryTracker::current_tag() const // NOLINT(readability-convert-member-functions-to-static)
  {
    return thread_local_memory_tag_stack()[thread_local_mem_tag_index()];
  }

  void MemoryTracker::dump_stats_to_file(rsl::string_view filepath)
  {
    MemoryTrackingStats stats = current_allocation_stats();

    debug_string_stream ss(rsl::io::openmode::in | rsl::io::openmode::out);

    for(count_t i = 0; i < stats.tracking_stats.usage_per_tag.size(); ++i)
    {
      const MemoryTag tag = static_cast<MemoryTag>(i);
      ss << rsl::format("{}: {} bytes\n", rsl::enum_refl::enum_name(tag), stats.tracking_stats.usage_per_tag[i]);
    }

    ss << "----------------------------\n";

    ss << rsl::format("Number of unique callstacks: {}\n", m_allocation_info_table.size());
    ss << "All sizes reported are inclusive. Meaning the size reported is the combined size of all the allocations using a particular callstack\n";
    ss << "\n";

    // copy the alloc callstacks as it's possible allocations occur while formatting or logging to file
    for(const auto& [callstack, alloc_info]: m_allocation_info_table)
    {
      ss << rsl::format("Count: {}\n", alloc_info.allocation_callstack.ref_count());
      ss << rsl::format("Size: {}\n", alloc_info.allocation_callstack.size());
      ss << rsl::format("Known Deleters: {}\n", alloc_info.deleter_callstacks.size());

      const ResolvedCallstack resolved_callstack(callstack);

      for(count_t i = 0; i < resolved_callstack.size(); ++i)
      {
        ss << rsl::format("{}\n", resolved_callstack[i]);
      }
    }

    const rsl::string_view content = ss.view();

    const rsl::time_point time_point = rsl::current_timepoint();
    rex::debug_string dated_filepath;
    dated_filepath += time_point.date().to_string_without_weekday();
    dated_filepath += "_";
    dated_filepath += time_point.time().to_string();
    dated_filepath += "_";
    dated_filepath += filepath;
    rsl::replace(dated_filepath.begin(), dated_filepath.end(), ':', '_');
    rsl::replace(dated_filepath.begin(), dated_filepath.end(), '/', '_');

    vfs::instance()->write_to_file(MountingPoint::Logs, dated_filepath, content.data(), content.length(), rex::AppendToFile::no);
  }

  MemoryAllocationStats MemoryTracker::current_tracking_stats()
  {
    const rsl::unique_lock lock(m_mem_tracking_mutex);
    MemoryAllocationStats stats{};
    stats.usage_per_tag = m_usage_per_tag;
    stats.used_memory = m_mem_usage.value();
    stats.max_used_memory = m_mem_usage.max_value();
    stats.num_alive_allocations = m_allocation_info_table.size();
    stats.num_total_allocations = m_num_total_allocations;

    return stats;
  }

  MemoryTrackingStats MemoryTracker::current_allocation_stats()
  {
    MemoryTrackingStats stats {};
    stats.tracking_stats = current_tracking_stats();
    stats.allocation_headers = m_allocation_headers;
    return stats;
  }

  MemoryTrackingStats MemoryTracker::get_pre_init_stats()
  {
    return get_stats_for_frame(-1); // allocations that happen before initialization have -1 as frame index
  }

  MemoryTrackingStats MemoryTracker::get_init_stats()
  {
    return get_stats_for_frame(0); // allocations that happen at initialization have 0 as frame index
  }

  MemoryTrackingStats MemoryTracker::get_stats_for_frame(card32 idx)
  {
    rsl::unique_lock lock(m_mem_tracking_mutex);
    const debug_vector<MemoryHeader*> alloc_headers = m_allocation_headers; // copy here on purpose as we don't want any race conditions when looping over it
    lock.unlock();

    MemoryTrackingStats stats {};
    stats.allocation_headers.reserve(alloc_headers.size());

    for(MemoryHeader* header: alloc_headers)
    {
      if(header->frame_index() == idx)
      {
        stats.allocation_headers.push_back(header);
        stats.tracking_stats.usage_per_tag[rsl::enum_refl::enum_integer(header->tag())] += header->size();
      }
      else if(header->frame_index() > idx)
      {
        break;
      }
    }

    return stats;
  }

  MemoryTagScope::MemoryTagScope(MemoryTag tag)
  {
    mem_tracker().push_tag(tag);
  }
  MemoryTagScope::~MemoryTagScope()
  {
    mem_tracker().pop_tag();
  }

  MemoryTracker& mem_tracker()
  {
    static MemoryTracker tracker;
    return tracker;
  }
} // namespace rex
