#pragma once

namespace rex
{
	// A debug interface should be the last resource destroyed
	// It acts as a safety check to validate all resources are destroyed
	class DebugInterface
	{
	public:
		virtual ~DebugInterface() = default;

		virtual void report_live_objects() = 0;
	private:

	};
}