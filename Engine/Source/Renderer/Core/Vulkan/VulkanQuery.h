#pragma once

#include "Renderer/Core/Query.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"
#include "VulkanResource.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanQuery class. Vulkan render side
    */
    class VulkanQuery : public Query//, public VulkanResource
    {
    public:

        VulkanQuery() = delete;
        VulkanQuery(VulkanQuery&) = delete;

        VulkanQuery(QueryType type, QueryRespose callback);
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
