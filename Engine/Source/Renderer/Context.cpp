#include "Context.h"
#include "Platform/Window.h"
#include "EmptyContext.h"
#include "Utils/Logger.h"

#ifdef VULKAN_RENDER
#   include "Vulkan/VulkanContext.h"
#endif //VULKAN_RENDER
#ifdef D3D_RENDER
#   include "D3D12/D3DGraphicContext.h"
#endif //D3D_RENDER

namespace v3d
{
namespace renderer
{

Context::Context() noexcept
    : m_frameCounter(0)
{
    memset(&m_backufferDescription, 0, sizeof(BackbufferDesc));
}

Context* Context::createContext(platform::Window* window, RenderType type, DeviceMask mask)
{
#ifdef PLATFORM_ANDROID
    if (type == RenderType::DirectXRender)
    {
        LOG_WARNING("Context::createContext type cant be %d for Android. Force switch to Vulkan", type);
        type = RenderType::VulkanRender;
    }
#endif //PLATFORM_ANDROID

    Context* context = nullptr;
    switch (type)
    {
    case RenderType::EmptyRender:
        context = new EmptyContext();
        break;

#ifdef VULKAN_RENDER
    case RenderType::VulkanRender:
        context = new vk::VulkanContext(window, mask);
        break;
#endif //VULKAN_RENDER

#ifdef D3D_RENDER
    case RenderType::DirectXRender:
        if (mask & DeviceMask::GraphicMask)
        {
            context = new dx3d::D3DGraphicContext(window);
        }
        else
        {
            LOG_ERROR("Context::createContext mask %x is not supported for this render type %u. Set default bit", mask, type);
            context = new dx3d::D3DGraphicContext(window);
        }
        break;
#endif //D3D_RENDER

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

const core::Dimension2D& Context::getBackbufferSize() const
{
    return m_backufferDescription._size;
}

Format Context::getBackbufferFormat() const
{
    return m_backufferDescription._format;
}

} //namespace renderer
} //namespace v3d
