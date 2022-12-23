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

VulkanQuery::VulkanQuery(QueryType type, QueryRespose callback)
    : Query(type, callback)
{
}

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER