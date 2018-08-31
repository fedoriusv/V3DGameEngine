#include "VulkanGraphicContext.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace renderer
{
namespace vk
{

VulkanGraphicContext::VulkanGraphicContext()
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
    return true;
}

void VulkanGraphicContext::destroy()
{
    LOG_DEBUG("VulkanGraphicContext::destroy");
}

} //namespace vk
} //namespace renderer
} //namespace v3d
