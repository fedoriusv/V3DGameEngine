#pragma once

#include "Common.h"
#include "Renderer/ShaderProperties.h"

#ifdef D3D_RENDER
#include "D3DConfiguration.h"
#include "D3DDescriptorSet.h"

namespace v3d
{
namespace renderer
{
namespace dx3d
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    using SignatureParameters = std::map<D3DBinding, u32, D3DBinding::Less>;

    /**
    * D3DRootSignature struct. DirectX Render side
    */
    struct D3DRootSignatureCreator
    {
    public:

        D3DRootSignatureCreator(const ShaderProgramDescription& desc);

        u32 m_hash;
        ID3DBlob* m_signature;

        SignatureParameters m_signatureParameters;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * D3DRootSignatureManager final class. DirectX Render side
    */

    class D3DRootSignatureManager
    {
    public:

        explicit D3DRootSignatureManager(ID3D12Device* device) noexcept;
        ~D3DRootSignatureManager();

        std::tuple<ID3D12RootSignature*, SignatureParameters> acquireRootSignature(const ShaderProgramDescription& desc);
        void removeAllRootSignatures();

    private:

        ID3D12Device* m_device;
        std::map<u32, ID3D12RootSignature*> m_rootSignatures;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace dx3d
} //namespace renderer
} //namespace v3d

#endif //D3D_RENDER
