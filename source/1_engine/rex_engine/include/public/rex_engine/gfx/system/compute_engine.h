#pragma once

#include "rex_engine/gfx/system/graphics_engine.h"
#include "rex_engine/gfx/system/compute_context.h"



namespace rex
{
  namespace gfx
  {
    // The compute engine is responsible for all compute commands on the gpu
    class ComputeEngine : public GraphicsEngine
    {
    public:
      ComputeEngine(ResourceStateTracker* globalResourceStateTracker);

      void new_frame();
      void end_frame();

    protected:
      // Prepare a new frame
      virtual void api_new_frame() = 0;
      // Finalise the frame
      virtual void api_end_frame() = 0;

    private:

    };
  }
}