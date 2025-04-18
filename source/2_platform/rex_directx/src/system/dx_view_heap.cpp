#include "rex_directx/system/dx_view_heap.h"

#include "rex_engine/diagnostics/assert.h"
#include "rex_engine/engine/casting.h"
#include "rex_engine/memory/pointer_math.h"

#include "rex_directx/resources/dx_sampler_2d.h"
#include "rex_engine/gfx/graphics.h"

namespace rex
{
  namespace gfx
  {
    DxViewHeap::DxViewHeap(const wrl::ComPtr<ID3D12DescriptorHeap>& viewHeap, const wrl::ComPtr<ID3D12Device1>& device, IsShaderVisible isShaderVisible)
			: m_view_heap(viewHeap)
			, m_device(device)
			, m_num_used_views(0)
			, m_view_size(0)
      , m_is_shader_visible(isShaderVisible)
    {
      const D3D12_DESCRIPTOR_HEAP_DESC desc = m_view_heap->GetDesc();
      m_view_heap_type                = desc.Type;
      m_num_views               = static_cast<s32>(desc.NumDescriptors);
      m_view_size               = static_cast<s32>(m_device->GetDescriptorHandleIncrementSize(m_view_heap_type)); // NOLINT(cppcoreguidelines-prefer-member-initializer)

      init_null_handle();
    }

    // Create a render target view and return a handle pointing to it
    DxResourceView DxViewHeap::create_rtv(ID3D12Resource* resource)
    {
      REX_ASSERT_X(m_view_heap_type == D3D12_DESCRIPTOR_HEAP_TYPE_RTV, "Trying to create a render target view from a view heap that's not configured to create render target views");
      DxResourceView rtv_handle = new_free_handle();

      return retarget_rtv(resource, rtv_handle);
    }
    // Retarget an existing rtv to a new resource
    DxResourceView& DxViewHeap::retarget_rtv(ID3D12Resource* resource, DxResourceView& rtv)
    {
      REX_ASSERT_X(m_view_heap_type == D3D12_DESCRIPTOR_HEAP_TYPE_RTV, "Trying to create a render target view from a view heap that's not configured to create render target views");

      D3D12_RENDER_TARGET_VIEW_DESC rtv_desc{};
      rtv_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
      rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

      m_device->CreateRenderTargetView(resource, &rtv_desc, rtv);

      return rtv;
    }

    // Create a depth stencil view and return a handle pointing to it
    DxResourceView DxViewHeap::create_dsv(ID3D12Resource* resource)
    {
      REX_ASSERT_X(m_view_heap_type == D3D12_DESCRIPTOR_HEAP_TYPE_DSV, "Trying to create a depth stencil view from a view heap that's not configured to create depth stencil views");

      // Valid formats for depth stencil view
      // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_depth_stencil_view_desc
      // DXGI_FORMAT_D16_UNORM
      // DXGI_FORMAT_D24_UNORM_S8_UINT
      // DXGI_FORMAT_D32_FLOAT
      // DXGI_FORMAT_D32_FLOAT_S8X24_UINT
      // DXGI_FORMAT_UNKNOWN

      D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc {};
      dsv_desc.Flags         = D3D12_DSV_FLAG_NONE;
      dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
      dsv_desc.Format        = resource->GetDesc().Format;

      DxResourceView dsv_handle = new_free_handle();
      m_device->CreateDepthStencilView(resource, &dsv_desc, dsv_handle);

      return dsv_handle;
    }
    // Create a constant buffer view and return a handle pointing to it
    DxResourceView DxViewHeap::create_cbv(ID3D12Resource* resource, rsl::memory_size size)
    {
      REX_ASSERT_X(m_view_heap_type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, "Trying to create a constant buffer view from a view heap that's not configured to create constant buffers views");

      // Create a new constant buffer view given the offsetted GPU address and the size of the constant buffer in bytes
      D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc;
      cbv_desc.BufferLocation = resource->GetGPUVirtualAddress();
      cbv_desc.SizeInBytes = static_cast<UINT>(size.size_in_bytes());

      DxResourceView cbv_handle = new_free_handle();
      m_device->CreateConstantBufferView(&cbv_desc, cbv_handle);

      return cbv_handle;
    }
    // Create a unordered access buffer view and return a handle pointing to it
    DxResourceView DxViewHeap::create_uav(ID3D12Resource* resource, rsl::memory_size size)
    {
      REX_ASSERT_X(m_view_heap_type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, "Trying to create a unordered access view from a view heap that's not configured to create unordered access views");

      D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc{};
      uav_desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
      uav_desc.Buffer.FirstElement = 0;
      uav_desc.Format = DXGI_FORMAT_R32_UINT;
      uav_desc.Buffer.NumElements = static_cast<UINT>(size / sizeof(d3d::format_byte_size(uav_desc.Format)));

      DxResourceView uav_handle = new_free_handle();
      m_device->CreateUnorderedAccessView(resource, nullptr, &uav_desc, uav_handle);

      return uav_handle;
    }

    // Create a shader resource view pointing to a texture and return a handle pointing to this view
    DxResourceView DxViewHeap::create_texture2d_srv(ID3D12Resource* resource)
    {
      REX_ASSERT_X(m_view_heap_type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, "Trying to create a constant buffer view from a view heap that's not configured to create constant buffers views");

      D3D12_RESOURCE_DESC resource_desc = resource->GetDesc();
      D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
      desc.Format = resource_desc.Format;
      desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
      desc.Texture2D.MipLevels = 1;
      desc.Texture2D.MostDetailedMip = 0;
      desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

      DxResourceView desc_handle = new_free_handle();
      m_device->CreateShaderResourceView(resource, &desc, desc_handle);
      return desc_handle;
    }

    // Create a 2D texture sampler
    rsl::unique_ptr<DxSampler2D> DxViewHeap::create_sampler2d(const SamplerDesc& desc)
    {
      REX_ASSERT_X(m_view_heap_type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, "Trying to create a sampler from a view heap that's not configured to create samplers");

      D3D12_SAMPLER_DESC sampler_desc = d3d::to_dx12(desc);
      DxResourceView desc_handle = new_free_handle();
      m_device->CreateSampler(&sampler_desc, desc_handle.cpu_handle());

      return rsl::make_unique<DxSampler2D>(desc_handle);
    }

    // Copy the given views into this heap
    rsl::unique_ptr<ResourceView> DxViewHeap::copy_views(const rsl::vector<const ResourceView*>& views)
    {
      if (views.empty())
      {
        return nullptr;
      }

      rsl::unique_ptr<DxResourceView> free_handle = rsl::make_unique<DxResourceView>(new_free_handle(views.size()));

      CD3DX12_CPU_DESCRIPTOR_HANDLE cpu_handle;
      for (const ResourceView* view : views)
      {
        cpu_handle = m_null_view.cpu_handle();
        // It's possible a null view is provided if the parameter has not been set yet
        if (view != nullptr)
        {
          const DxResourceView* src_handle = d3d::to_dx12(view);
          cpu_handle = src_handle->cpu_handle();
          m_device->CopyDescriptorsSimple(1, free_handle->cpu_handle(), cpu_handle, m_view_heap_type);
        }

        (*free_handle)++;
      }

      return free_handle;
    }

    // Return the internal wrapped view heap
    ID3D12DescriptorHeap* DxViewHeap::dx_object()
    {
      return m_view_heap.Get();
    }

    // Reset the view heap
    // This will cause new view to be allocated from the beginning of the heap
    // this does not destroy existing views, 
    // it only repoints the allocating start back to the beginning of the heap
    void DxViewHeap::clear()
    {
      // We always have 1 view in user for the null view
      m_num_used_views = 1;
    }

    // Return a handle pointing to the start of the view heap
    DxResourceView DxViewHeap::new_free_handle(s32 numDescriptors)
    {
      DxResourceView handle = my_start_handle();
      handle += m_num_used_views;
      m_num_used_views += numDescriptors;
      return handle;
    }
    // Return a handle pointing to a free bit of memory in the view heap
    DxResourceView DxViewHeap::my_start_handle()
    {
      D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle = m_view_heap->GetCPUDescriptorHandleForHeapStart();
      D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle{};

      if (m_is_shader_visible)
      {
        gpu_handle = m_view_heap->GetGPUDescriptorHandleForHeapStart();
      }

      return DxResourceView(cpu_handle, gpu_handle, m_view_heap_type, m_view_size, m_is_shader_visible);
    }
    // Create a handle pointing to no resource
    void DxViewHeap::init_null_handle()
    {
      m_null_view = new_free_handle();
      switch (m_view_heap_type)
      {
      case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
      {
        D3D12_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc{};
        shader_resource_view_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        shader_resource_view_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        shader_resource_view_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        shader_resource_view_desc.Texture2D.MipLevels = 1;
        shader_resource_view_desc.Texture2D.MostDetailedMip = 0;
        shader_resource_view_desc.Texture2D.ResourceMinLODClamp = 0.0f;
        m_device->CreateShaderResourceView(rsl::Nullptr<ID3D12Resource>, &shader_resource_view_desc, m_null_view); break;
      }
      default: break;
      }
    }

  } // namespace gfx
} // namespace rex