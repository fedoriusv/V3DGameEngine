#pragma once

#include "Object.h"
#include "Render.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class ShaderProgram;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief SamplerDesc. The struct describes sampler parameters
    */
    struct SamplerDesc
    {
        /**
        * @brief SamplerDesc struct. Size 24 bytes
        */
        math::Vector4D          _borderColor; //TODO: color class with 4 bytes
        f32                     _lodBias;

        SamplerAnisotropic      _anisotropic : 5;
        SamplerWrap             _wrapU : 3;
        SamplerWrap             _wrapV : 3;
        SamplerWrap             _wrapW : 3;
        SamplerFilter           _filter : 2;
        CompareOperation        _compareOp : 3;
        u32                     _enableCompOp : 1;

        u32                     _padding : 12;
    };

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
        const math::Vector4D& getBorderColor() const;

        void setFiltering(SamplerFilter filter);
        void setWrap(SamplerWrap uvw);
        void setWrap(SamplerWrap u, SamplerWrap v, SamplerWrap w = SamplerWrap::TextureWrap_Repeat);
        void setAnisotropic(SamplerAnisotropic level);
        void setLodBias(f32 value);
        void setCompareOp(CompareOperation op);
        void setEnableCompareOp(bool enable);
        void setBorderColor(const math::Vector4D& color);

    private:

        /**
        * @briefSamplerState constructor. Used for creating sampler.
        * Private method. Use createObject interface inside CommandList class to call.
        */
        SamplerState() noexcept;

        /**
        * @brief SamplerState constructor. Used for creating sampler.
        * Private method. Use createObject interface inside CommandList class to call.
        *
        * @param SamplerFilter filter [required]
        * @param SamplerAnisotropic aniso [required]
        */
        SamplerState(SamplerFilter filter, SamplerAnisotropic aniso) noexcept;

        SamplerDesc m_samplerDesc;

        friend ShaderProgram;
    };

    inline SamplerFilter SamplerState::getFiltering() const
    {
        return m_samplerDesc._filter;
    }

    inline SamplerWrap SamplerState::getWrapU() const
    {
        return m_samplerDesc._wrapU;
    }

    inline SamplerWrap SamplerState::getWrapV() const
    {
        return m_samplerDesc._wrapV;
    }

    inline SamplerWrap SamplerState::getWrapW() const
    {
        return m_samplerDesc._wrapW;
    }

    inline SamplerAnisotropic SamplerState::getAnisotropic() const
    {
        return m_samplerDesc._anisotropic;
    }

    inline CompareOperation SamplerState::getCompareOp() const
    {
        return m_samplerDesc._compareOp;
    }

    inline const math::Vector4D& SamplerState::getBorderColor() const
    {
        return m_samplerDesc._borderColor;
    }

    inline bool SamplerState::isEnableCompareOp() const
    {
        return m_samplerDesc._enableCompOp;
    }

    inline void SamplerState::setFiltering(SamplerFilter filter)
    {
        m_samplerDesc._filter = filter;
    }

    inline void SamplerState::setWrap(SamplerWrap uvw)
    {
        m_samplerDesc._wrapU = uvw;
        m_samplerDesc._wrapV = uvw;
        m_samplerDesc._wrapW = uvw;
    }

    inline void SamplerState::setWrap(SamplerWrap u, SamplerWrap v, SamplerWrap w)
    {
        m_samplerDesc._wrapU = u;
        m_samplerDesc._wrapV = v;
        m_samplerDesc._wrapW = w;
    }

    inline void SamplerState::setAnisotropic(SamplerAnisotropic level)
    {
        m_samplerDesc._anisotropic = level;
    }

    inline void SamplerState::setLodBias(f32 value)
    {
        m_samplerDesc._lodBias = value;
    }

    inline void SamplerState::setCompareOp(CompareOperation op)
    {
        m_samplerDesc._compareOp = op;
    }

    inline void SamplerState::setEnableCompareOp(bool enable)
    {
        m_samplerDesc._enableCompOp = enable;
    }

    inline void SamplerState::setBorderColor(const math::Vector4D& color)
    {
        m_samplerDesc._borderColor = color;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Sampler : public utils::ResourceID<Sampler, u8>
    {
    protected:

        Sampler() = default;
        virtual ~Sampler() = default;

        virtual bool create() = 0;
        virtual void destroy() = 0;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
