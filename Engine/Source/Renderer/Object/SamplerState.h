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
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class ShaderProgram;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief SamplerState class. Game side
    */
    class SamplerState : public Object
    {
    public:

        SamplerState() = delete;
        SamplerState(const SamplerState&) = delete;

        ~SamplerState();

        SamplerFilter         getFiltering() const;
        SamplerWrap           getWrapU() const;
        SamplerWrap           getWrapV() const;
        SamplerWrap           getWrapW() const;
        SamplerAnisotropic    getAnisotropic() const;
        CompareOperation      getCompareOp() const;
        bool                  isEnableCompareOp() const;
        const core::Vector4D& getBorderColor() const;

        void setFiltering(SamplerFilter filter);
        void setWrap(SamplerWrap uvw);
        void setWrap(SamplerWrap u, SamplerWrap v, SamplerWrap w = SamplerWrap::TextureWrap_Repeat);
        void setAnisotropic(SamplerAnisotropic level);
        void setLodBias(f32 value);
        void setCompareOp(CompareOperation op);
        void setEnableCompareOp(bool enable);
        void setBorderColor(const core::Vector4D& color);

    private:

        /**
        * @briefSamplerState constructor. Used for creating sampler.
        * Private method. Use createObject interface inside CommandList class to call.
        */
        explicit SamplerState(renderer::CommandList& cmdList) noexcept;

        /**
        * @brief SamplerState constructor. Used for creating sampler.
        * Private method. Use createObject interface inside CommandList class to call.
        *
        * @param SamplerFilter filter [required]
        * @param SamplerAnisotropic aniso [required]
        */
        explicit SamplerState(renderer::CommandList& cmdList, SamplerFilter filter, SamplerAnisotropic aniso) noexcept;

        CommandList& m_cmdList;
        friend CommandList;

        void destroySamplers(const std::vector<Sampler*>& samplers);

        SamplerDescription m_samplerDesc;
        ObjectTracker<Sampler> m_trackerSampler;

        friend ShaderProgram;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
