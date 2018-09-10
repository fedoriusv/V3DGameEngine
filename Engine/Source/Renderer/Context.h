#pragma once

#include "Common.h"
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

        //test
        virtual void clearColor(const core::Vector4D& color) = 0;

        virtual void setViewport(const core::Rect32& viewport) = 0;

        virtual class Image* createImage() const = 0;

    protected:

        virtual bool initialize() = 0;
        virtual void destroy() = 0;

        RenderType m_renderType;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
