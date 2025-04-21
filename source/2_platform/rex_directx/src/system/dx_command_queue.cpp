#include "rex_directx/system/dx_command_queue.h"
#include "rex_directx/diagnostics/dx_call.h"

#include "rex_directx/system/dx_render_context.h"
#include "rex_directx/system/dx_compute_context.h"

namespace rex
{
  namespace gfx
  {
    DxCommandQueue::DxCommandQueue(GraphicsEngineType type, const wrl::ComPtr<ID3D12CommandQueue>& DxCommandQueue, rsl::unique_ptr<DxFence> fence)
      : CommandQueue(type)
      , m_command_queue(DxCommandQueue)
      , m_fence(rsl::move(fence))
      , m_fence_event()
    {
    }

    DxCommandQueue::~DxCommandQueue()
    {
      flush();
    }

    // Halt the cpu until the fence value is reached
    void DxCommandQueue::cpu_wait()
    {
      m_fence->inc(this);
      m_fence->wait_for_me();
    }
    // Halt the gpu until the fence value is reached
    void DxCommandQueue::gpu_wait(SyncInfo& sync_info)
    {
      ID3D12Fence* fence = d3d::to_dx12(sync_info.fence_object())->dx_object();
      m_command_queue->Wait(fence, sync_info.fence_val());
    }

    ScopedPoolObject<SyncInfo> DxCommandQueue::execute_context(GraphicsContext* ctx, WaitForFinish waitForFinish)
    {
      ID3D12GraphicsCommandList* cmdlist = cmdlist_from_ctx(ctx);

      cmdlist->Close();
      ID3D12CommandList* base_cmdlist = cmdlist;
      m_command_queue->ExecuteCommandLists(1, &base_cmdlist);
      ctx->end_profile_event();

      u64 old_fence_val = m_fence->inc(this);

      if (waitForFinish)
      {
        flush();
      }

      return create_sync_info(old_fence_val, m_fence.get());
    }

    u64 DxCommandQueue::gpu_fence_value() const
    {
      return m_fence->gpu_value();
    }

    ID3D12CommandQueue* DxCommandQueue::dx_object()
    {
      return m_command_queue.Get();
    }

    ID3D12GraphicsCommandList* DxCommandQueue::cmdlist_from_ctx(GraphicsContext* ctx) const
    {
      REX_ASSERT_X(ctx->type() == type(), "Trying to execute a context on a command queue that doesn't match its type. ctx: {} queue: {}", rsl::enum_refl::enum_name(ctx->type()), rsl::enum_refl::enum_name(type()));
      switch (type())
      {
      case GraphicsEngineType::Render:   return static_cast<DxRenderContext*>(ctx)->dx_cmdlist();
      case GraphicsEngineType::Compute:  return static_cast<DxComputeContext*>(ctx)->dx_cmdlist();
      }

      REX_ASSERT("Unknown command type used for command queue. Cannot get command list of context");
      return nullptr;
    }

  } // namespace gfx
} // namespace rex