#pragma once

#include "Context.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class EmptyContext final : public Context
    {
    public:

        EmptyContext();
        ~EmptyContext();

        void beginFrame() override;
        void endFrame() override;
        void presentFrame() override;

        void setViewport(const core::Rect32& viewport) override;

        //SwapchainTexture* getBackbuffer() const override;

        Image* createImage(TextureTarget target, renderer::ImageFormat m_format, core::Dimension3D dimension, u32 m_mipmapLevel,
            s16 filter, TextureAnisotropic anisotropicLevel, TextureWrap wrap) const override;

    private:

        bool initialize() override;
        void destroy() override;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
