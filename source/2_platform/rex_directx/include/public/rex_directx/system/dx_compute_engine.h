#pragma once

#include "rex_engine/gfx/system/compute_engine.h"
#include "rex_directx/system/dx_command_allocator.h"
#include "rex_directx/utility/dx_util.h"
#include "rex_directx/system/dx_compute_context.h"
#include "rex_directx/system/dx_gal.h"



namespace rex
{
  namespace gfx
  {
    class DxComputeEngine : public gfx::ComputeEngine
    {
    public:
      DxComputeEngine(ResourceStateTracker* globalResourceStateTracker);

      // Initialize all the resources required for the compute engine
      void init() override;

    protected:
      // Prepare a new frame
      void api_new_frame() final;
      // Finalise the frame
      void api_end_frame() final;

      // Allocate a copy context
      rsl::unique_ptr<GraphicsContext> allocate_new_context(CommandAllocator* alloc) override;
    };
  }
}