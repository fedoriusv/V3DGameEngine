#include "Context.h"
#include "Platform/Window.h"

#include "EmptyContext.h"
#include "Vulkan/VulkanGraphicContext.h"

#include "Utils/Logger.h"

namespace v3d
{
namespace renderer
{

Context::Context() noexcept
    : m_frameCounter(0)
{
    memset(&m_backufferDescription, 0, sizeof(Backbuffer));
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

void Context::destroyContext(Context* context)
{
    context->destroy();
    delete context;
}

Context::RenderType Context::getRenderType() const
{
    return m_renderType;
}

u64 Context::getCurrentFrameIndex() const
{
    return m_frameCounter;
}

} //namespace renderer
} //namespace v3d
