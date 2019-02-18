#pragma once

#include "Common.h"
#include "Object.h"
#include "Utils/Observable.h"
#include "Renderer/TextureProperties.h"
#include "Renderer/ObjectTracker.h"
#include "Renderer/CommandList.h"

namespace v3d
{
namespace renderer
{
     /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * SamplerState class. Game side
    */
    class SamplerState : public Object, public utils::Observer
    {
    public:

        ~SamplerState() {};
        SamplerState(const SamplerState &) = delete;

        SamplerFilter         getMinFilter() const;
        SamplerFilter         getMagFilter() const;
        SamplerWrap           getWrap() const;
        SamplerAnisotropic    getAnisotropic() const;

    private:

        SamplerState(renderer::CommandList& cmdList) noexcept;

        renderer::CommandList& m_cmdList;
        friend renderer::CommandList;

        void handleNotify(utils::Observable* ob) override;

        u32                           m_filter;
        renderer::SamplerAnisotropic  m_anisotropicLevel;
        renderer::SamplerWrap         m_wrap;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
