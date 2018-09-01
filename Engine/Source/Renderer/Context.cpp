#include "Context.h"
#include "Platform/Window.h"
#include "Utils/Logger.h"

#include "EmptyContext.h"
#include "Vulkan/VulkanGraphicContext.h"



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

Context* Context::createContext(const platform::Window* window,  RenderType type, DeviceMask mask)
{
    Context* context = nullptr;
    switch (type)
    {
    case RenderType::EmptyRender:
        context = new EmptyContext();
        break;

    case RenderType::VulkanRender:
        if (mask & DeviceMask::GraphicMask)
        {
            context = new vk::VulkanGraphicContext(window);
        }
        else
        {
            LOG_ERROR("Context::createContext mask %x is not supported for this render type %u. Set default bit", mask, type);
            context = new vk::VulkanGraphicContext(window);
        }
        break;

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
