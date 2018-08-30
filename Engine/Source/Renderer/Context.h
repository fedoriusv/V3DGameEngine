#pragma once
#include "Common.h"

namespace v3d
{
namespace renderer
{
    class EmptyContext;

    class Context //no clonable
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

        static Context* createContext(RenderType type);

        virtual void beginFrame() = 0;
        virtual void endFrame() = 0;
        virtual void presentFrame() = 0;

        virtual bool initialize() = 0;
        virtual void destroy() = 0;

    protected:

        RenderType m_renderType;
    };

} //namespace renderer
} //namespace v3d
