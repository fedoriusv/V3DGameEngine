#pragma once

#include "Common.h"
#include "Object.h"
#include "Utils/Observable.h"

#include "SamplerProperties.h"
#include "ObjectTracker.h"
#include "CommandList.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Sampler;
    class ShaderProgram;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief SamplerState class. Game side.
    * Describes how texture will be filtered, etc.
    */
    class SamplerState : public Object
    {
    public:

        /**
        * @brief SamplerState destructor
        */
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

        SamplerState() = delete;
        SamplerState(const SamplerState&) = delete;

        CommandList& m_cmdList;
        friend CommandList;

        void destroySamplers(const std::vector<Sampler*>& samplers);

        SamplerDescription m_samplerDesc;
        ObjectTracker<Sampler> m_trackerSampler;

        friend ShaderProgram;
    };

    inline SamplerFilter SamplerState::getFiltering() const
    {
        return m_samplerDesc._desc._filter;
    }

    inline SamplerWrap SamplerState::getWrapU() const
    {
        return m_samplerDesc._desc._wrapU;
    }

    inline SamplerWrap SamplerState::getWrapV() const
    {
        return m_samplerDesc._desc._wrapV;
    }

    inline SamplerWrap SamplerState::getWrapW() const
    {
        return m_samplerDesc._desc._wrapW;
    }

    inline SamplerAnisotropic SamplerState::getAnisotropic() const
    {
        return m_samplerDesc._desc._anisotropic;
    }

    inline CompareOperation SamplerState::getCompareOp() const
    {
        return m_samplerDesc._desc._compareOp;
    }

    inline const core::Vector4D& SamplerState::getBorderColor() const
    {
        return m_samplerDesc._desc._borderColor;
    }

    inline bool SamplerState::isEnableCompareOp() const
    {
        return m_samplerDesc._desc._enableCompOp;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
