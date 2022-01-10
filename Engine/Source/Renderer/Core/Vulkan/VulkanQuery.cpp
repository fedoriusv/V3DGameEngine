#include "VulkanQuery.h"

#include "Utils/Logger.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"
#include "VulkanDebug.h"
#include "VulkanContext.h"

namespace v3d
{
namespace renderer
{
namespace vk
{

VkQueryType VulkanQueryPool::convertQueryTypeToVkQuery(QueryType type)
{
    switch (type)
    {
    case QueryType::Occlusion:
        return VK_QUERY_TYPE_OCCLUSION;
        
    case QueryType::TimeStamp:
        return VK_QUERY_TYPE_TIMESTAMP;

    default:
        ASSERT(false, "unknown type");
    }

    return VK_QUERY_TYPE_OCCLUSION;
}


VulkanQueryPool::VulkanQueryPool(VkDevice device, QueryType type, u32 size) noexcept
    : QueryPool(type, size)
    , m_device(device)
    , m_pool(VK_NULL_HANDLE)
{
    LOG_DEBUG("VulkanQueryPool::VulkanQueryPool constructor %llx", this);
}

VulkanQueryPool::~VulkanQueryPool()
{
    LOG_DEBUG("VulkanQueryPool::VulkanQueryPool destructor %llx", this);
    ASSERT(m_pool, "must be nullptr");
}

bool VulkanQueryPool::create()
{
    VkQueryPoolCreateInfo queryPoolCreateInfo = {};
    queryPoolCreateInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    queryPoolCreateInfo.pNext = nullptr;
    queryPoolCreateInfo.flags = 0;
    queryPoolCreateInfo.pipelineStatistics = 0;
    queryPoolCreateInfo.queryType = VulkanQueryPool::convertQueryTypeToVkQuery(m_type);
    queryPoolCreateInfo.queryCount = m_size;

    VkResult result = VulkanWrapper::CreateQueryPool(m_device, &queryPoolCreateInfo, VULKAN_ALLOCATOR, &m_pool);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("VulkanQueryPool::create vkCreateQueryPool is failed. Error: %s", ErrorString(result).c_str());
        return false;
    }

    return true;
}

void VulkanQueryPool::destroy()
{
    if (m_pool)
    {
        VulkanWrapper::DestroyQueryPool(m_device, m_pool, VULKAN_ALLOCATOR);
        m_pool = VK_NULL_HANDLE;
    }
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER