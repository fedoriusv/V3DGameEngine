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

VulkanQuery::VulkanQuery(QueryType type, const QueryRespose& callback, [[maybe_unused]] const std::string& name) noexcept
    : Query(type, callback, name)
{
    LOG_DEBUG("VulkanQuery::VulkanQuery constructor %llx", this);
}

VulkanQuery::~VulkanQuery()
{
    LOG_DEBUG("VulkanQuery::~VulkanQuery destructor %llx", this);
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER