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

        enum class RenderType
        {
            EmptyRender,
            VulkanRender,
            //....
        };

        Context();
        virtual ~Context();

        static Context* createContext(const platform::Window* window, RenderType type);

        virtual void beginFrame() = 0;
        virtual void endFrame() = 0;
        virtual void presentFrame() = 0;

        virtual bool initialize() = 0;
        virtual void destroy() = 0;

    protected:

        RenderType m_renderType;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
