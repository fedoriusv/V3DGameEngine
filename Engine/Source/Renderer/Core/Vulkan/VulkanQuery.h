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

    class VulkanRenderQueryManager;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanQuery class. Vulkan render side
    */
    class VulkanQuery : public Query
    {
    public:

        VulkanQuery() = delete;
        VulkanQuery(VulkanQuery&) = delete;

        VulkanQuery(QueryType type, const QueryRespose& callback, [[maybe_unused]] const std::string& name = "") noexcept;
        ~VulkanQuery();

        friend VulkanRenderQueryManager;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
