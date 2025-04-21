#pragma once

#include "rex_engine/engine/types.h"
#include "rex_engine/engine/globals.h"
#include "rex_engine/gfx/core/texture_format.h"

namespace rex
{
  namespace gfx
  {
    class CommandQueue;

    struct ImGuiDevice
    {
      CommandQueue* command_queue; // This is needed to create swapchains on imgui spawned windows
      s32 max_num_frames_in_flight;     // This is needed to initialize the swapchain on imgui spawned windows
      TextureFormat rtv_format; // This is needed to initialize the swapchain on imgui spawned windows
    };

    namespace imgui_device
    {
			void init(globals::GlobalUniquePtr<ImGuiDevice> device);
			ImGuiDevice* instance();
			void shutdown();
		}
  }
}