#include "rex_engine/gfx/imgui/imgui_device.h"

namespace rex
{
  namespace gfx
  {
    globals::GlobalUniquePtr<ImGuiDevice> g_imgui_device;
    void init(globals::GlobalUniquePtr<ImGuiDevice> device)
    {
      g_imgui_device = rsl::move(device);
    }
    ImGuiDevice* instance()
    {
      return g_imgui_device.get();
    }
    void shutdown()
    {
      g_imgui_device.reset();
    }
  }
}