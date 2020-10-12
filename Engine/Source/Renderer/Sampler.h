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
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Context;
    class SamplerManager;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Sampler base class. Render side
    */
    class Sampler : public RenderObject<Sampler>, public utils::Observable
    {
    public:

        /**
        * @brief SamplerInfo struct
        */
        struct SamplerInfo
        {
            SamplerInfo() noexcept
                : _tracker(nullptr)
            {
            }

            SamplerDescription      _desc;
            ObjectTracker<Sampler>* _tracker;
        };

        explicit Sampler(const SamplerDescription& desc) noexcept;
        virtual ~Sampler();

        virtual bool create() = 0;
        virtual void destroy() = 0;

    protected:

        const SamplerDescription m_desc;
        friend SamplerManager;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief SamplerManager class
    */
    class SamplerManager final : utils::Observer
    {
    public:

        SamplerManager() = delete;

        explicit SamplerManager(Context *context) noexcept;
        ~SamplerManager();

        void handleNotify(utils::Observable* ob) override;

        Sampler* acquireSampler(const SamplerDescription& samplerDesc);
        bool removeSampler(Sampler* sampler);
        void clear();

    private:

        Context* m_context;
        std::unordered_map<SamplerDescription, Sampler*, SamplerDescription::Hash, SamplerDescription::Compare> m_samplerList;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
