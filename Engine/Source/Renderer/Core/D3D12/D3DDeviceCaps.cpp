#include "D3DDeviceCaps.h"
#include "Utils/Logger.h"

#ifdef D3D_RENDER
#include "D3DDebug.h"
#include "D3DImage.h"

namespace v3d
{
namespace renderer
{
namespace dx3d
{

void D3DDeviceCaps::initialize(IDXGIAdapter1* adapter, ID3D12Device* device)
{
    {
        rootSignatureVersion.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
        HRESULT result = device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &rootSignatureVersion, sizeof(rootSignatureVersion));
        if (FAILED(result))
        {
            rootSignatureVersion.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
        }
    }

    {
        HRESULT result = device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &featureOptions, sizeof(featureOptions));
        if (FAILED(result))
        {
            LOG_WARNING("D3DDeviceCaps::initialize: CheckFeatureSupport D3D12_FEATURE_D3D12_OPTIONS is failed. Error %s", D3DDebug::stringError(result).c_str());
        }
    }

    {
        HRESULT result = device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS3, &featureOptions3, sizeof(featureOptions3));
        if (FAILED(result))
        {
            LOG_WARNING("D3DDeviceCaps::initialize: CheckFeatureSupport D3D12_FEATURE_D3D12_OPTIONS3 is failed. Error %s", D3DDebug::stringError(result).c_str());
        }
    }

    {
        HRESULT result = device->CheckFeatureSupport(D3D12_FEATURE_ARCHITECTURE, &featureArchitecture, sizeof(featureArchitecture));
        if (FAILED(result))
        {
            LOG_WARNING("D3DDeviceCaps::initialize: CheckFeatureSupport D3D12_FEATURE_ARCHITECTURE is failed. Error %s", D3DDebug::stringError(result).c_str());
        }
    }

    for (u32 i = 0; i < Format::Format_Count; ++i)
    {
        D3D12_FEATURE_DATA_FORMAT_SUPPORT formatSupport = {};
        formatSupport.Format = D3DImage::convertImageFormatToD3DFormat((Format)i);
        if (formatSupport.Format == DXGI_FORMAT_UNKNOWN)
        {
            continue;
        }

        HRESULT result = device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(formatSupport));
        if (FAILED(result))
        {
            LOG_WARNING("D3DDeviceCaps::initialize: CheckFeatureSupport D3D12_FEATURE_FORMAT_SUPPORT is failed. Error %s", D3DDebug::stringError(result).c_str());
        }
        else
        {
            m_imageFormatSupport[i][TilingType_Optimal]._supportSampled = (formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE) ? true : false;
            m_imageFormatSupport[i][TilingType_Optimal]._supportAttachment = ((formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_RENDER_TARGET) || (formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL)) ? true : false;
            m_imageFormatSupport[i][TilingType_Optimal]._supportMip = (formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_MIP) ? true : false;
            m_imageFormatSupport[i][TilingType_Optimal]._supportStorage = (formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_TYPED_UNORDERED_ACCESS_VIEW) ? true : false;
            m_imageFormatSupport[i][TilingType_Optimal]._supportResolve = (formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_MULTISAMPLE_RESOLVE) ? true : false;
        }
    }

    {
        D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = {};
        shaderModel.HighestShaderModel = D3D_SHADER_MODEL_6_7; //set max
        HRESULT result = device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel));
        if (SUCCEEDED(result))
        {
            supportedShaderModel = shaderModel.HighestShaderModel;
        }
    }


    //set features

    if ((supportedShaderModel >= D3D_SHADER_MODEL_6_1) && (featureOptions3.ViewInstancingTier >= D3D12_VIEW_INSTANCING_TIER_2) && (D3D12_MAX_VIEW_INSTANCE_COUNT >= 6U)) //TODO D3D12_MAX_VIEW_INSTANCE_COUNT remove condition after api update
    {
        supportMultiview = true;
    }

    globalComandListAllocator = false; //TODO has memory leak when command lists reset
    ASSERT(!immediateSubmitUpload, "not impl");

#if D3D_DEBUG
    {
        DXGI_ADAPTER_DESC desc = {};
        ASSERT(SUCCEEDED(adapter->GetDesc(&desc)), "failed");

        LOG_INFO("D3DDeviceCaps::initialize: Device ID: %u", desc.DeviceId);
        LOG_INFO("D3DDeviceCaps::initialize: Vendor: %u", desc.VendorId);
        LOG_INFO("D3DDeviceCaps::initialize: Revision: %u", desc.Revision);
    }
#endif
}

} //namespace dx3d
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER


