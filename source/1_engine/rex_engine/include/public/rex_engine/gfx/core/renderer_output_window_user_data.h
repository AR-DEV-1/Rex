#pragma once

#include "rex_engine/engine/types.h"

namespace rex
{
  namespace gfx
  {
    // Wrapper around some user data that's relevant for the renderer
    struct OutputWindowUserData
    {
      void* primary_display_handle; // this is HWND on Windows

      s32 window_width;     
      s32 window_height;

      s32 refresh_rate;         // The number of refreshes per second, 60 fps has a refresh rate of 60
      s32 max_frames_in_flight; // The max number of frames we can have prepared for rendering

      bool windowed;            // Flag storing that we're in windowed mode (and not in fullscreen
    };
  } // namespace gfx
} // namespace rex