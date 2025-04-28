#include "rex_directx/dxgi/dx_adapter.h"
#include "rex_directx/dxgi/includes.h"
#include "rex_directx/diagnostics/dx_call.h"
#include "rex_directx/system/dx_device.h"
#include "rex_engine/diagnostics/logging/log_macros.h"
#include "rex_engine/engine/types.h"
#include "rex_engine/text_processing/text_processing.h"
#include "rex_std/bonus/memory/memory_size.h"
#include "rex_std/bonus/string.h"
#include "rex_std/string.h"
#include "rex_directx/system/dx_feature_level.h"

#include <cstdlib>
#include <wrl/client.h>

#include "rex_engine/system/video_driver.h"

namespace
{
  const rsl::array g_expected_feature_levels = { D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_12_0, D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_12_1 };

  //-------------------------------------------------------------------------
  template <typename DXGIAdapterDesc>
  rex::GpuDescription convert_description(const DXGIAdapterDesc& dxgiDesc)
  {
    rex::GpuDescription desc;

    desc.name = rsl::small_stack_string(rex::to_multibyte(dxgiDesc.Description, desc.name.max_size()));
    desc.vendor_name = rex::gpu_vendor_to_string(dxgiDesc.VendorId);

    desc.vendor_id = static_cast<rex::GpuVendor>(dxgiDesc.VendorId);
    desc.device_id = dxgiDesc.DeviceId;

    desc.dedicated_video_memory = rsl::memory_size(dxgiDesc.DedicatedVideoMemory);
    desc.dedicated_system_memory = rsl::memory_size(dxgiDesc.DedicatedSystemMemory);
    desc.shared_system_memory = rsl::memory_size(dxgiDesc.SharedSystemMemory);

    return desc;
  }

  //-------------------------------------------------------------------------
  rex::GpuDescription get_description(const rex::wrl::ComPtr<IDXGIAdapter>& adapter)
  {
    rex::GpuDescription desc;

    // MSDN says that you shouldn't use dxgi 1.0 and dxgi 1.1
    // in the same application.
    // so if the version is 1 or higher, we use dxgi 1.1
    rex::wrl::ComPtr<IDXGIAdapter1> adapter_1;
    adapter.As<IDXGIAdapter1>(&adapter_1);
    if (adapter_1)
    {
      DXGI_ADAPTER_DESC1 dxgi_desc;
      adapter_1->GetDesc1(&dxgi_desc);
      desc = convert_description(dxgi_desc);
    }
    else
    {
      DXGI_ADAPTER_DESC dxgi_desc;
      adapter->GetDesc(&dxgi_desc);
      desc = convert_description(dxgi_desc);
    }

    return desc;
  }
} // namespace

namespace rex
{
  namespace gfx
  {
    namespace dxgi
    {
      DEFINE_LOG_CATEGORY(LogDxAdapter);

      //-------------------------------------------------------------------------
      Adapter::Adapter(wrl::ComPtr<IDXGIAdapter>&& adapter, u32 version)
        : DxgiObject(rsl::move(adapter), version)
        , m_description(::get_description(dx_object()))
      {
        m_highest_feature_level = query_highest_feature_level();
      }

      //-------------------------------------------------------------------------
      const GpuDescription& Adapter::description() const
      {
        return m_description;
      }

      //-------------------------------------------------------------------------
      rsl::unique_ptr<DxDevice> Adapter::create_device() const
      {
        wrl::ComPtr<ID3D12Device1> d3d_device;
        if (DX_FAILED(D3D12CreateDevice(dx_object(), m_highest_feature_level, IID_PPV_ARGS(&d3d_device))))
        {
          REX_ERROR(LogDxAdapter, "Failed to create DX12 Device on {}", description().name);
          return nullptr;
        }

        return rsl::make_unique<DxDevice>(d3d_device, m_highest_feature_level, this);
      }

      //-------------------------------------------------------------------------
      D3D_FEATURE_LEVEL Adapter::query_highest_feature_level()
      {
        // backwards looping as it's checking for a minimum feature level
        for (auto it = g_expected_feature_levels.crbegin(); it != g_expected_feature_levels.crend(); ++it)
        {
          const D3D_FEATURE_LEVEL feature_level = *it;
          if (SUCCEEDED(D3D12CreateDevice(dx_object(), feature_level, __uuidof(ID3D12Device), nullptr)))
          {
            return feature_level;
          }
        }

        REX_ASSERT("At least D3D_FEATURE_LEVEL_12_0 has to be supported for DirectX 12!");

        // If the compiler doesn't recognise D3D_FEATURE_LEVEL_1_0_CORE
        // Make sure you're using windows SDK 10.0.18362.0 or later
        return D3D_FEATURE_LEVEL_1_0_CORE;
      }

    } // namespace dxgi
  }
} // namespace rex
