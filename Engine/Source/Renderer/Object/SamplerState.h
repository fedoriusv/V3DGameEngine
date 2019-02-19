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
    */
    class SamplerState : public Object, public utils::Observer
    {
    public:

        SamplerState() = delete;
        SamplerState(const SamplerState &) = delete;
        ~SamplerState();

        SamplerFilter         getMinFilter() const;
        SamplerFilter         getMagFilter() const;
        SamplerWrap           getWrap() const;
        SamplerAnisotropic    getAnisotropic() const;

        //TODO sets

    private:

        SamplerState(renderer::CommandList& cmdList, SamplerFilter min, SamplerFilter mag) noexcept;

        renderer::CommandList& m_cmdList;
        friend renderer::CommandList;

        void handleNotify(utils::Observable* ob) override;
        void destroySamplers(const std::vector<Sampler*>& samplers);

        SamplerDescription            m_samplerDesc;

        u32                           m_filter;
        renderer::SamplerAnisotropic  m_anisotropicLevel;
        renderer::SamplerWrap         m_wrap;

        ObjectTracker<Sampler>       m_trackerSampler;

        friend ShaderProgram;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
