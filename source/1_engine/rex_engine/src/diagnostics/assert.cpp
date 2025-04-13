#include "rex_engine/diagnostics/assert.h"

#include "rex_engine/diagnostics/logging/log_macros.h"
#include "rex_engine/diagnostics/logging/log_verbosity.h"
#include "rex_engine/diagnostics/stacktrace.h"
#include "rex_engine/memory/memory_types.h"
#include "rex_engine/engine/defines.h"
#include "rex_std/bonus/types.h"
#include "rex_engine/diagnostics/debug.h"

namespace rex
{
  DEFINE_LOG_CATEGORY(LogAssert);

  namespace internal
  {
    // We make sure we write to debug output as well as it's possible an assert is raised before init or after shutdown
    // If this happens we still want to get some kind of debug information, so we make sure we log to the debug output window
    void debug_log_and_error(rsl::string_view err)
    {
      rex::output_debug_string(err);
      REX_ERROR(LogAssert, err);
    }
  }

  debug_vector<AssertContext>& contexts()
  {
    thread_local debug_vector<AssertContext> contexts;
    return contexts;
  }

  void rex_assert(const rsl::fmt_stack_string& msg)
  {
    REX_UNUSED_PARAM(msg);

#ifndef REX_NO_LOGGING
    thread_local static bool is_processing_assert = false;
    if(!is_processing_assert)
    {

      is_processing_assert = true;
      internal::debug_log_and_error(rsl::format("Assert Raised: {}", msg));

      if (contexts().size() > 0)
      {
        internal::debug_log_and_error(rsl::format("Assert contexts:"));
        internal::debug_log_and_error(rsl::format("----------------"));

        for (const AssertContext& context : contexts())
        {
          internal::debug_log_and_error(rsl::format("{}", context.msg()));
          internal::debug_log_and_error(rsl::format("[traceback] {}", rsl::to_string(context.source_location())));
        }

        internal::debug_log_and_error(rsl::format("----------------"));
      }

      const ResolvedCallstack callstack(current_callstack());

      for(count_t i = 0; i < callstack.size(); ++i)
      {
        internal::debug_log_and_error(rsl::format("{}", callstack[i]));
      }
    }
    else
    {
      // if this is hit, an assert occurred while processing another one.
      // to avoid circular dependency, we break here if there's a debugger attached
      REX_DEBUG_BREAK();
    }
#endif
  }

  void push_assert_context(const rsl::fmt_stack_string& msg, rsl::source_location sourceLoc)
  {
    contexts().emplace_back(msg, sourceLoc);
  }
  void pop_assert_context()
  {
    contexts().pop_back();
  }

  AssertContext::AssertContext(const rsl::fmt_stack_string& msg, rsl::source_location sourceLoc)
      : m_msg(msg)
      , m_source_location(sourceLoc)
  {
  }

  const rsl::fmt_stack_string& AssertContext::msg() const
  {
    return m_msg;
  }
  const rsl::source_location& AssertContext::source_location() const
  {
    return m_source_location;
  }

  AssertContextScope::AssertContextScope(const rsl::fmt_stack_string& msg, rsl::source_location sourceLoc)
  {
    push_assert_context(msg, sourceLoc);
  }

  AssertContextScope::~AssertContextScope()
  {
    pop_assert_context();
  }
} // namespace rex