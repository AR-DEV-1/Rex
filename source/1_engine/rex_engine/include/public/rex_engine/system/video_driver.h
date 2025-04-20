#pragma once

#include "rex_std/string_view.h"
#include "rex_std/bonus/string.h"
#include "rex_engine/engine/types.h"
#include "rex_engine/system/gpu_vendor.h"

namespace rex
{
	struct VideoDriverInfo
	{
		rsl::string device_description;
		rsl::string provider_name;
		rsl::string internal_driver_version;
		rsl::string user_driver_version;
		rsl::string driver_date;
		GpuVendor vendor_id = GpuVendor::Unknown;
	};

	VideoDriverInfo query_video_driver(rsl::string_view deviceName);
	rsl::string nvidia_unified_version(rsl::string_view internalVersion);

}

#ifdef REX_PLATFORM_WINDOWS
#include "rex_engine/platform/win/system/win_video_driver.h"
#endif