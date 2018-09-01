#include "VulkanGraphicContext.h"
#include "VulkanDebug.h"
#include "Utils/Logger.h"

#ifdef VULKAN_RENDER
#   ifdef PLATFORM_WINDOWS
#       pragma comment(lib, "vulkan-1.lib")
#   endif //PLATFORM_WINDOWS
namespace v3d
{
namespace renderer
{
namespace vk
{

const std::vector<const c8*> k_instanceExtensionsList = 
{
    VK_KHR_SURFACE_EXTENSION_NAME,
    VK_KHR_DISPLAY_EXTENSION_NAME,
#ifdef VK_KHR_win32_surface
    VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
#ifdef VK_KHR_android_surface
    VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
#endif
#ifdef VK_KHR_xlib_surface
    VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
#endif
#if VULKAN_DEBUG
    VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
#endif //VULKAN_DEBUG
};

const std::vector<const c8*> k_deviceExtensions =
{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME,
    VK_EXT_DEBUG_MARKER_EXTENSION_NAME,
};

VulkanGraphicContext::VulkanGraphicContext(const platform::Window* window)
{
    LOG_DEBUG("VulkanGraphicContext created this %llx", this);
    m_renderType = RenderType::VulkanRender;
}

VulkanGraphicContext::~VulkanGraphicContext()
 {
     LOG_DEBUG("~VulkanGraphicContext destructor this %llx", this);
 }

void VulkanGraphicContext::beginFrame()
{
    LOG_DEBUG("VulkanGraphicContext::beginFrame");
}

void VulkanGraphicContext::endFrame()
{
    LOG_DEBUG("VulkanGraphicContext::endFrame");
}

void VulkanGraphicContext::presentFrame()
{
    LOG_DEBUG("VulkanGraphicContext::presentFrame");
}

bool VulkanGraphicContext::initialize()
{
    LOG_DEBUG("VulkanGraphicContext::initialize");

    if (!VulkanGraphicContext::createInstance())
    {
        ASSERT(false, "createInstance failed");
        LOG_FATAL("VulkanGraphicContext::createInstance failed");
        return false;
    }

    if (!VulkanGraphicContext::createDevice())
    {
        ASSERT(false, "createDevice failed");
        LOG_FATAL("VulkanGraphicContext::createDevice failed");
        return false;
    }


    return true;
}

void VulkanGraphicContext::destroy()
{
    LOG_DEBUG("VulkanGraphicContext::destroy");
}

bool VulkanGraphicContext::createInstance()
{
    VkApplicationInfo applicationInfo = {};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pNext = nullptr;
    applicationInfo.pApplicationName = s_vulkanApplicationName.c_str();
    applicationInfo.applicationVersion = 1;
    applicationInfo.pEngineName = s_vulkanApplicationName.c_str();
    applicationInfo.engineVersion = 1;
    applicationInfo.apiVersion = VK_MAKE_VERSION(VULKAN_VERSION_MAJOR, VULKAN_VERSION_MINOR, VULKAN_VERSION_PATCH);

    std::vector<std::string> supportedExtensions;
    VulkanDeviceCaps::listOfInstanceExtensions(supportedExtensions);

#if VULKAN_DEBUG
    for (auto iter = supportedExtensions.cbegin(); iter != supportedExtensions.cend(); ++iter)
    {
        LOG_INFO("VulkanGraphicContext::createInstance: extention: [%s]", (*iter).c_str());
    }
#endif //VULKAN_DEBUG
    std::vector<const c8*> enabledExtensions;
    for (auto extentionName = k_instanceExtensionsList.cbegin(); extentionName != k_instanceExtensionsList.cend(); ++extentionName)
    {
        bool found = false;
        for (auto iter = supportedExtensions.cbegin(); iter != supportedExtensions.cend(); ++iter)
        {
            if (!(*iter).compare(*extentionName))
            {
                LOG_INFO("VulkanGraphicContext::createInstance: enable extention: [%s]", (*iter).c_str());
                enabledExtensions.push_back(*extentionName);
                found = true;
                break;
            }
        }

        if (!found)
        {
            LOG_ERROR("VulkanGraphicContext::createInstance: extention [%s] is not supported", *extentionName);
        }
    }

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext = nullptr;
    instanceCreateInfo.flags = 0;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;
    instanceCreateInfo.enabledExtensionCount = static_cast<u32>(enabledExtensions.size());
    instanceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();

    std::vector<const c8*> layerNames;
#if VULKAN_VALIDATION_LAYERS_CALLBACK
    for (auto layerName = VulkanDebug::s_validationLayerNames.cbegin(); layerName < VulkanDebug::s_validationLayerNames.cend(); ++layerName)
    {
        if (VulkanDebug::checkLayerIsSupported(*layerName))
        {
            LOG_INFO("VulkanGraphicContext::createInstance: enable validation layer: [%s]", *layerName);
            layerNames.push_back(*layerName);
        }
    }
#endif //VULKAN_VALIDATION_LAYERS_CALLBACK

#if VULKAN_RENDERDOC_LAYER
    const c8* renderdocLayerName = "VK_LAYER_RENDERDOC_Capture";
    if (VulkanDebug::checkLayerIsSupported(renderdocLayerName))
    {
        LOG_INFO("VulkanGraphicContext::createInstance: enable layer: [%s]", renderdocLayerName);
        layerNames.push_back(renderdocLayerName);
    }
#endif //VULKAN_RENDERDOC_LAYER

    instanceCreateInfo.enabledLayerCount = static_cast<u32>(layerNames.size());
    instanceCreateInfo.ppEnabledLayerNames = layerNames.data();

    VkResult result = VulkanWrapper::CreateInstance(&instanceCreateInfo, nullptr, &m_deviceInfo._instance);
    if (result != VK_SUCCESS)
    {
        LOG_FATAL("VulkanGraphicContext::createInstance: vkCreateInstance error %s", ErrorString(result).c_str());
        return false;
    }

    return true;
}

bool VulkanGraphicContext::createDevice()
{

    return false;
}

} //namespace vk
} //namespace renderer
} //namespace v3d

#endif //VULKAN_RENDER
