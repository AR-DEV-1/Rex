#include "rex_engine/gfx/system/compute_engine.h"

namespace rex
{
  namespace gfx
  {
    ComputeEngine::ComputeEngine(ResourceStateTracker* globalResourceStateTracker)
      : GraphicsEngine(GraphicsEngineType::Compute, globalResourceStateTracker)
    {}

    void ComputeEngine::new_frame()
    {
      api_new_frame();
    }
    void ComputeEngine::end_frame()
    {
      api_end_frame();
    }
  }
}