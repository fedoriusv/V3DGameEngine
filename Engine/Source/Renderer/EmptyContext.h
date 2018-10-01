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

        virtual void clearBackbuffer(const core::Vector4D & color) override;

        void setViewport(const core::Rect32& viewport) override;

        Image* createImage(TextureTarget target, renderer::ImageFormat format, core::Dimension3D dimension, u32 mipmapLevel,
            s16 filter, TextureAnisotropic anisotropicLevel, TextureWrap wrap) const override;

        Image* createAttachmentImage(renderer::ImageFormat format, core::Dimension3D dimension, TextureSamples samples,
            s16 filter, TextureAnisotropic anisotropicLevel, TextureWrap wrap) const override;

        Framebuffer* createFramebuffer() override;

    private:

        bool initialize() override;
        void destroy() override;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
