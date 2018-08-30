#include "Context.h"
#include "Utils/Logger.h"
#include "EmptyContext.h"


namespace v3d
{
namespace renderer
{

Context::Context()
{
}

Context::~Context()
{
}

Context* Context::createContext(RenderType type)
{
    Context* context = nullptr;
    switch (type)
    {
    case RenderType::EmptyRender:
        context = new EmptyContext();
        break;

    //case RenderType::VulkanRender:
    //    //
    //    break;

    default:
        ASSERT(false, "Render type is not implemented");
    }

    if (!context->initialize())
    {
        LOG_FATAL("Context::createContext can't initialize");

        delete context;
        return nullptr;
    }

    LOG_INFO("Context::createContext Initialize is done");
    return context;
}

} //namespace renderer
} //namespace v3d
