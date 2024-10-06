#include "Device.h"
#include "Utils/Logger.h"

#ifdef VULKAN_RENDER
#   include "Vulkan/VulkanDevice.h"
#endif //VULKAN_RENDER
#ifdef D3D_RENDER
#   include "D3D12/D3DGraphicContext.h"
#endif //D3D_RENDER

namespace v3d
{
namespace renderer
{

Device::Device() noexcept
    : m_renderType(RenderType::EmptyRender)
{
}

Device* Device::createDevice(RenderType type, DeviceMaskFlags mask)
{
    Device* render = nullptr;
    switch (type)
    {
    case RenderType::EmptyRender:
        break;

#ifdef VULKAN_RENDER
    case RenderType::VulkanRender:
        render = V3D_NEW(vk::VulkanDevice, memory::MemoryLabel::MemoryRenderCore)(mask);
        break;
#endif //VULKAN_RENDER

#ifdef D3D_RENDER
    case RenderType::DirectXRender:
        break;
#endif //D3D_RENDER

    default:
        ASSERT(false, "Render type is not implemented");
    }

    if (!render->initialize())
    {
        LOG_FATAL("RenderDevice::createRenderDevice can't initialize");

        V3D_FREE(render, memory::MemoryLabel::MemoryRenderCore);
        return nullptr;
    }

    LOG_INFO("RenderDevice::createRenderDevice Initialize is done");
    return render;
}

void Device::destroyDevice(Device* device)
{
    device->destroy();
    V3D_FREE(device, memory::MemoryLabel::MemoryRenderCore);
}

} //namespace renderer
} //namespace v3d
