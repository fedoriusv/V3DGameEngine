#pragma once

#include "Common.h"
#include "Utils/Observable.h"
#include "Renderer/SamplerProperties.h"
#include "Renderer/PipelineStateProperties.h"

namespace v3d
{
namespace renderer
{
    class Context;
    class SamplerManager;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Sampler base class. Render side
    */
    class Sampler : public utils::Observable
    {
    public:

        struct SamplerInfo
        {
            SamplerFilter       _mag;
            SamplerFilter       _min;
            SamplerAnisotropic  _aniso;
            SamplerWrap         _wrap[3];
            f32                 _mipBias;
            f32                 _minLod;
            f32                 _maxLod;

            CompareOperation    _compareOp;
            bool                _enableCompOp;
        };

        Sampler() noexcept;
        virtual ~Sampler();

        virtual bool create(const SamplerInfo& info) = 0;
        virtual void destroy() = 0;

    private:

        u32 m_key;

        friend SamplerManager;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

     /**
    * SamplerManager class
    */
    class SamplerManager final : utils::Observer
    {
    public:

        SamplerManager() = delete;

        explicit SamplerManager(Context *context) noexcept;
        ~SamplerManager();

        void handleNotify(utils::Observable* ob) override;

        Sampler* acquireSampler(const Sampler::SamplerInfo& samplerInfo);
        bool removeSampler(const Sampler::SamplerInfo& samplerInfo);
        bool removeSampler(Sampler* sampler);
        void clear();

    private:

        union SamplerDescription
        {
            SamplerDescription() {}

            Sampler::SamplerInfo  _info;
            u32                   _hash;
        };

        Context*                   m_context;
        std::map<u32, Sampler*>    m_samplerList;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
