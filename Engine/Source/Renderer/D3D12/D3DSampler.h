#pragma once

#include "Common.h"
#include "Renderer/Object/SamplerState.h"

#ifdef D3D_RENDER
#include "D3DConfiguration.h"
#include "D3DResource.h"

namespace v3d
{
namespace renderer
{
namespace dx3d
{

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * D3DSampler final class. DirectX Render side
    */
    class D3DSampler final : public Sampler, public D3DResource
    {
    public:

        static D3D12_FILTER convertSamplerFilterToD3D(SamplerFilter min, SamplerFilter mag);
        static D3D12_TEXTURE_ADDRESS_MODE convertSamplerWrapToAddressModeD3D(SamplerWrap wrap);
        static D3D12_COMPARISON_FUNC convertSamplerCompareOpD3D(CompareOperation op);

        D3DSampler() noexcept;
        D3DSampler(const D3DSampler&) = delete;
        ~D3DSampler() = default;

        bool create(const SamplerDescription& info) override;
        void destroy() override;

        const D3D12_SAMPLER_DESC& getDesc() const;

    private:

        D3D12_SAMPLER_DESC m_desc;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace dx3d
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER