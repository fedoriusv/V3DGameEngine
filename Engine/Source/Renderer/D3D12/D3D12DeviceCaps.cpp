#include "D3D12DeviceCaps.h"

#ifdef VULKAN_RENDER



namespace v3d
{
namespace renderer
{
namespace dx3d
{

void D3DDeviceCaps::initialize(ID3D12Device* device)
{
    rootSignatureVersion.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
    HRESULT result = device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &rootSignatureVersion, sizeof(rootSignatureVersion));
    if (FAILED(result))
    {
        rootSignatureVersion.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }
}

} //namespace v3d12
} //namespace renderer
} //namespace v3d

#endif //D3D_RENDER


