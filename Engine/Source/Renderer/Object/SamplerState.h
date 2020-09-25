#pragma once

#include "Common.h"
#include "Object.h"
#include "Utils/Observable.h"
#include "Renderer/SamplerProperties.h"
#include "Renderer/ObjectTracker.h"
#include "Renderer/CommandList.h"

namespace v3d
{
namespace renderer
{
    class ShaderProgram;

     /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * SamplerState class. Game side
    *\n
    * Constructor param:
    SamplerFilter mag
    SamplerFilter min
    SamplerAnisotropic aniso
    *\n
    * Constructor param:
    SamplerFilter filter
    SamplerAnisotropic aniso
    */
    class SamplerState : public Object
    {
    public:

        SamplerState() = delete;
        SamplerState(const SamplerState &) = delete;

        ~SamplerState();

        SamplerFilter         getMinFilter() const;
        SamplerFilter         getMagFilter() const;
        SamplerWrap           getWrapU() const;
        SamplerWrap           getWrapV() const;
        SamplerWrap           getWrapW() const;
        SamplerAnisotropic    getAnisotropic() const;
        CompareOperation      getCompareOp() const;
        const core::Vector4D& getBorderColor() const;
        bool                  isEnableCompareOp() const;

        void setMinFilter(SamplerFilter filter);
        void setMagFilter(SamplerFilter filter);
        void setWrap(SamplerWrap uvw);
        void setWrap(SamplerWrap u, SamplerWrap v, SamplerWrap w = SamplerWrap::TextureWrap_Repeat);
        void setAnisotropic(SamplerAnisotropic level);
        void setLodBias(f32 value);
        void setCompareOp(CompareOperation op);
        void setEnableCompareOp(bool enable);
        void setBorderColor(const core::Vector4D& color);

    private:

        explicit SamplerState(renderer::CommandList& cmdList) noexcept;
        explicit SamplerState(renderer::CommandList& cmdList, SamplerFilter filter, SamplerAnisotropic aniso) noexcept;
        explicit SamplerState(renderer::CommandList& cmdList, SamplerFilter mag, SamplerFilter min, SamplerAnisotropic aniso) noexcept;

        renderer::CommandList& m_cmdList;
        friend renderer::CommandList;

        void destroySamplers(const std::vector<Sampler*>& samplers);

        SamplerDescription m_samplerDesc;
        ObjectTracker<Sampler> m_trackerSampler;

        friend ShaderProgram;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
