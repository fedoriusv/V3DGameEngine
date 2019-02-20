#pragma once

#include "Common.h"
#include "Utils/Observable.h"
#include "Renderer/SamplerProperties.h"
#include "Renderer/PipelineStateProperties.h"
#include "ObjectTracker.h"

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
    class Sampler : public RenderObject<Sampler>, public utils::Observable
    {
    public:

        struct SamplerInfo
        {
            SamplerInfo() noexcept
                : _tracker(nullptr)
            {
            }

            union SamplerDesc
            {
                SamplerDesc() noexcept
                {
                    memset(this, 0, sizeof(SamplerDesc));
                }

                SamplerDescription    _desc;
                u64                   _hash;
            };
            SamplerDesc             _value;
            ObjectTracker<Sampler>* _tracker;
        };

        Sampler() noexcept;
        virtual ~Sampler();

        virtual bool create(const SamplerDescription& info) = 0;
        virtual void destroy() = 0;

    private:

        u64 m_key;

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

        Sampler* acquireSampler(const SamplerDescription& samplerInfo);
        bool removeSampler(Sampler* sampler);
        void clear();

    private:

        Context*                   m_context;
        std::map<u64, Sampler*>    m_samplerList;
        //TODO hash map
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
