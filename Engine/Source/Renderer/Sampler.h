#pragma once

#include "Common.h"
#include "Utils/Observable.h"

namespace v3d
{
namespace renderer
{
    class Context;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Sampler base class. Render side
    */
    class Sampler : public utils::Observable
    {
    public:

        Sampler() noexcept;
        virtual ~Sampler();

        virtual bool create() = 0;
        virtual void destroy() = 0;
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
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
