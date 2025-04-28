#include "rex_directx/system/dx_resource_heap.h"

#include "rex_directx/diagnostics/dx_call.h"

#include "rex_directx/utility/d3dx12.h"
#include "rex_engine/diagnostics/log.h"

namespace rex
{
  namespace gfx
  {
    DEFINE_LOG_CATEGORY(LogResourceHeap);

    // A resource heap currently acts like a stack based allocator
    // This is likely not sufficient enough in the future, but it'll do for now

    DxResourceHeap::DxResourceHeap(const wrl::ComPtr<ID3D12Heap>& heap, const wrl::ComPtr<ID3D12Device1>& device)
        : m_heap(heap)
        , m_device(device)
        , m_used_memory(0)
        , m_memory_limit(0)
    {
      const D3D12_HEAP_DESC heap_desc = m_heap->GetDesc();
      m_memory_limit            = static_cast<s64>(heap_desc.SizeInBytes);
    }

    wrl::ComPtr<ID3D12Resource> DxResourceHeap::create_buffer(rsl::memory_size size, s32 alignment)
    {
      return create_buffer(size, alignment, D3D12_RESOURCE_FLAG_NONE);
    }
    wrl::ComPtr<ID3D12Resource> DxResourceHeap::create_unordered_access_buffer(rsl::memory_size size, s32 alignment)
    {
      return create_buffer(size, alignment, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
    }

    wrl::ComPtr<ID3D12Resource> DxResourceHeap::create_texture2d(DXGI_FORMAT format, s32 width, s32 height)
    {
      CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(format, width, height);
      desc.MipLevels = 1;
      const D3D12_RESOURCE_ALLOCATION_INFO alloc_info = m_device->GetResourceAllocationInfo(0, 1, &desc);
      REX_ASSERT_X(can_fit_allocation(alloc_info), "Trying to allocate {} bytes which would overrun resource heap of {} bytes", alloc_info.SizeInBytes, m_memory_limit);

      wrl::ComPtr<ID3D12Resource> texture;
      if (DX_FAILED(m_device->CreatePlacedResource(m_heap.Get(), m_used_memory, &desc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&texture))))
      {
        REX_ERROR(LogResourceHeap, "Failed to create 2D texture");
        return nullptr;
      }

      m_used_memory += static_cast<s64>(alloc_info.SizeInBytes);
      return texture;
    }

    wrl::ComPtr<ID3D12Resource> DxResourceHeap::create_depth_stencil_buffer(DXGI_FORMAT format, s32 width, s32 height, const ClearStateDesc& clearStateDesc)
    {
      CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(format, width, height);
      desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
      desc.MipLevels = 1; // We do not want to generate mips for a depth stencil buffer
      const D3D12_RESOURCE_ALLOCATION_INFO alloc_info = m_device->GetResourceAllocationInfo(0, 1, &desc);
      REX_ASSERT_X(can_fit_allocation(alloc_info), "Trying to allocate {} bytes which would overrun resource heap of {} bytes", alloc_info.SizeInBytes, m_memory_limit);

      D3D12_CLEAR_VALUE clear_value{};
      clear_value.DepthStencil.Depth = clearStateDesc.depth;
      clear_value.DepthStencil.Stencil = clearStateDesc.stencil;
      clear_value.Format = format;

      wrl::ComPtr<ID3D12Resource> texture;
      if (DX_FAILED(m_device->CreatePlacedResource(m_heap.Get(), m_used_memory, &desc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clear_value, IID_PPV_ARGS(&texture))))
      {
        REX_ERROR(LogResourceHeap, "Failed to create 2D texture");
        return nullptr;
      }

      m_used_memory += static_cast<s64>(alloc_info.SizeInBytes);
      return texture;
    }

    bool DxResourceHeap::can_fit_allocation(const D3D12_RESOURCE_ALLOCATION_INFO& alloc_info) const
    {
      return static_cast<s64>(m_used_memory.size_in_bytes() + alloc_info.SizeInBytes) < m_memory_limit;
    }

    wrl::ComPtr<ID3D12Resource> DxResourceHeap::create_buffer(rsl::memory_size size, s32 alignment, D3D12_RESOURCE_FLAGS flags)
    {
      auto desc = CD3DX12_RESOURCE_DESC::Buffer(size, flags, alignment);
      const D3D12_RESOURCE_ALLOCATION_INFO alloc_info = m_device->GetResourceAllocationInfo(0, 1, &desc);
      REX_ASSERT_X(can_fit_allocation(alloc_info), "Trying to allocate {} bytes which would overrun resource heap of {} bytes", alloc_info.SizeInBytes, m_memory_limit);

      wrl::ComPtr<ID3D12Resource> buffer;
      if (DX_FAILED(m_device->CreatePlacedResource(m_heap.Get(), m_used_memory, &desc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&buffer))))
      {
        REX_ERROR(LogResourceHeap, "Failed to create buffer");
        return nullptr;
      }

      // Your application can forgo using GetResourceAllocationInfo for buffer resources (D3D12_RESOURCE_DIMENSION_BUFFER). 
      // Buffers have the same size on all adapters, which is merely the smallest multiple of 64KB that's greater or equal to D3D12_RESOURCE_DESC::Width.
      // See: https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12device-getresourceallocationinfo(uint_uint_constd3d12_resource_desc)#:~:text=Your%20application%20can%20forgo%20using%20GetResourceAllocationInfo%20for%20buffer%20resources%20(D3D12_RESOURCE_DIMENSION_BUFFER).%20Buffers%20have%20the%20same%20size%20on%20all%20adapters%2C%20which%20is%20merely%20the%20smallest%20multiple%20of%2064KB%20that%27s%20greater%20or%20equal%20to%20D3D12_RESOURCE_DESC%3A%3AWidth.
      m_used_memory += static_cast<s64>(alloc_info.SizeInBytes);
      return buffer;
    }

  } // namespace gfx
} // namespace rex