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
    * TextureSampler class. Game side
    */
    class TextureSampler : public Object //??????
    {
    public:

        ~TextureSampler() {};
        TextureSampler(const TextureSampler &) = delete;

        TextureFilter         getMinFilter() const;
        TextureFilter         getMagFilter() const;
        TextureWrap           getWrap() const;
        TextureAnisotropic    getAnisotropic() const;

    private:

        TextureSampler(renderer::CommandList& cmdList) noexcept;

        renderer::CommandList& m_cmdList;
        friend renderer::CommandList;

        s16                           m_filter;
        renderer::TextureAnisotropic  m_anisotropicLevel;
        renderer::TextureWrap         m_wrap;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
