#pragma once

#include "Common.h"
#include "Object/Texture.h"
#include "Utils/NonCopyable.h"

namespace v3d
{
namespace platform
{
    class Window;
} //namespace platform

namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Image;
    class SwapchainTexture;

    class Context : public utils::NonCopyable
    {

    public:

        enum DeviceMask
        {
            GraphicMask = 0x1,
            ComputeMask = 0x2,
            TransferMask = 0x4
        };

        enum class RenderType
        {
            EmptyRender,
            VulkanRender,
        };

        Context();
        virtual ~Context();

        static Context* createContext(const platform::Window* window, RenderType type, DeviceMask mask = DeviceMask::GraphicMask);

        virtual void beginFrame() = 0;
        virtual void endFrame() = 0;
        virtual void presentFrame() = 0;

        virtual void setViewport(const core::Rect32& viewport) = 0;

        //virtual SwapchainTexture* getBackbuffer() const = 0;

    protected:

        friend Texture2D;

        virtual Image* createImage(TextureTarget target, renderer::ImageFormat m_format, core::Dimension3D dimension, u32 m_mipmapLevel,
            s16 filter, TextureAnisotropic anisotropicLevel, TextureWrap wrap) const = 0;

        virtual bool initialize() = 0;
        virtual void destroy() = 0;

        //std::vector<SwapchainTexture*> m_backbuffers;
        RenderType m_renderType;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
