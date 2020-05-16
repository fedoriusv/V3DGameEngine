#include "D3DDeviceCaps.h"
#include "Utils/Logger.h"

#ifdef D3D_RENDER
#include "D3DDebug.h"

namespace v3d
{
namespace renderer
{
namespace dx3d
{

void D3DDeviceCaps::initialize(ID3D12Device* device)
{
    invertZ = true;

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
}

} //namespace dx3d
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER


