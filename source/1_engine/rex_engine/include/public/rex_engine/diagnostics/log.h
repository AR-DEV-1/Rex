#pragma once

#include "rex_engine/diagnostics/logging/log_macros.h"

DEFINE_LOG_CATEGORY(LogEngine);

namespace rex
{
	rsl::stack_string<char8, 256> project_log_path();
}