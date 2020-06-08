#include "D3DSampler.h"

#ifdef D3D_RENDER
namespace v3d
{
namespace renderer
{
namespace dx3d
{

D3D12_FILTER D3DSampler::convertSamplerFilterToD3D(SamplerFilter min, SamplerFilter mag)
{
    switch (mag)
    {
    case SamplerFilter::SamplerFilter_Nearest:
        switch (min)
        {
            case SamplerFilter::SamplerFilter_Nearest:
            default:
                return D3D12_FILTER_MIN_MAG_MIP_POINT;

            case SamplerFilter::SamplerFilter_Bilinear:
                return D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;

            case SamplerFilter::SamplerFilter_Trilinear:
            case SamplerFilter::SamplerFilter_Cubic:
                return D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
        }
        break;

    case SamplerFilter::SamplerFilter_Bilinear:
    case SamplerFilter::SamplerFilter_Trilinear:
    case SamplerFilter::SamplerFilter_Cubic:
    {
        switch (min)
        {
        case SamplerFilter::SamplerFilter_Nearest:
            return D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;

        case SamplerFilter::SamplerFilter_Bilinear:
            return D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;

        case SamplerFilter::SamplerFilter_Trilinear:
        case SamplerFilter::SamplerFilter_Cubic:
            return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        }
        break;
    }
    }

    return D3D12_FILTER_MIN_MAG_MIP_POINT;
}

D3D12_TEXTURE_ADDRESS_MODE D3DSampler::convertSamplerWrapToAddressModeD3D(SamplerWrap wrap)
{
    switch (wrap)
    {
        case SamplerWrap::TextureWrap_Repeat:
            return D3D12_TEXTURE_ADDRESS_MODE_WRAP;

        case SamplerWrap::TextureWrap_MirroredRepeat:
            return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;

        case SamplerWrap::TextureWrap_ClampToEdge:
            return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

        case SamplerWrap::TextureWrap_ClampToBorder:
            return D3D12_TEXTURE_ADDRESS_MODE_BORDER;

        case SamplerWrap::TextureWrap_MirroredClampToEdge:
            return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;

        default:
            ASSERT(false, "mode not found");

    }
    return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
}

D3D12_COMPARISON_FUNC D3DSampler::convertSamplerCompareOpD3D(CompareOperation op)
{
    switch(op)
    {
    case CompareOperation::CompareOp_Never:
        return D3D12_COMPARISON_FUNC_NEVER;

    case CompareOperation::CompareOp_Less:
        return D3D12_COMPARISON_FUNC_LESS;

    case CompareOperation::CompareOp_Equal:
        return D3D12_COMPARISON_FUNC_EQUAL;

    case CompareOperation::CompareOp_LessOrEqual:
        return D3D12_COMPARISON_FUNC_LESS_EQUAL;

    case CompareOperation::CompareOp_Greater:
        return D3D12_COMPARISON_FUNC_GREATER;

    case CompareOperation::CompareOp_NotEqual:
        return D3D12_COMPARISON_FUNC_NOT_EQUAL;

    case CompareOperation::CompareOp_GreaterOrEqual:
        return D3D12_COMPARISON_FUNC_GREATER_EQUAL;

    case CompareOperation::CompareOp_Always:
        return D3D12_COMPARISON_FUNC_ALWAYS;

    default:
        ASSERT(false, "op not found");
    }

    return D3D12_COMPARISON_FUNC_NEVER;
}

bool D3DSampler::create(const SamplerDescription& info)
{
    m_desc = {};
    m_desc.Filter = D3DSampler::convertSamplerFilterToD3D(info._minFilter, info._magFilter);
    m_desc.AddressU = D3DSampler::convertSamplerWrapToAddressModeD3D(info._wrapU);
    m_desc.AddressV = D3DSampler::convertSamplerWrapToAddressModeD3D(info._wrapV);
    m_desc.AddressW = D3DSampler::convertSamplerWrapToAddressModeD3D(info._wrapW);
    m_desc.MipLODBias = info._lodBias;
    m_desc.MaxAnisotropy = static_cast<UINT>(info._anisotropic);
    m_desc.ComparisonFunc = (info._enableCompOp) ? D3DSampler::convertSamplerCompareOpD3D(info._compareOp) : D3D12_COMPARISON_FUNC_NEVER;
    m_desc.BorderColor[0] = 0.0f;
    m_desc.BorderColor[1] = 0.0f;
    m_desc.BorderColor[2] = 0.0f;
    m_desc.BorderColor[3] = 0.0f;
    m_desc.MinLOD = 0.0f;
    m_desc.MaxLOD = FLT_MAX;

    return true;
}

void D3DSampler::destroy()
{
    //nothing
}

const D3D12_SAMPLER_DESC& D3DSampler::getDesc() const
{
    return m_desc;
}

} //namespace dx3d
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER