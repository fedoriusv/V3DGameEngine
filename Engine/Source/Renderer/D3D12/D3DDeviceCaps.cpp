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

void D3DDeviceCaps::initialize(ID3D12Device* device)
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

    for (u32 i = 0; i < Format::Format_Count; ++i)
    {
        D3D12_FEATURE_DATA_FORMAT_SUPPORT featureData = {};
        featureData.Format = D3DImage::convertImageFormatToD3DFormat((Format)i);
        if (featureData.Format == DXGI_FORMAT_UNKNOWN)
        {
            continue;
        }

        HRESULT result = device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &featureData, sizeof(featureData));
        if (FAILED(result))
        {
            LOG_WARNING("D3DDeviceCaps::initialize: CheckFeatureSupport D3D12_FEATURE_FORMAT_SUPPORT is failed. Error %s", D3DDebug::stringError(result).c_str());
        }
        else
        {
            m_imageFormatSupport[i][TilingType_Optimal]._supportSampled = (featureData.Support1 & D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE) ? true : false;
            m_imageFormatSupport[i][TilingType_Optimal]._supportAttachment = ((featureData.Support1 & D3D12_FORMAT_SUPPORT1_RENDER_TARGET) || (featureData.Support1 & D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL)) ? true : false;
            m_imageFormatSupport[i][TilingType_Optimal]._supportMip = (featureData.Support1 & D3D12_FORMAT_SUPPORT1_MIP) ? true : false;
        }
    }

    globalComandListAllocator = false; //TODO has memory leak when command lists reset
    ASSERT(!immediateSubmitUpload, "not impl");
}

} //namespace dx3d
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER


