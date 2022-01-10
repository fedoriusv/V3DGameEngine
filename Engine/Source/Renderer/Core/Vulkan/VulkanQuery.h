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

    class VulkanQueryPool final : public QueryPool, public VulkanResource
    {
    public:

        static VkQueryType convertQueryTypeToVkQuery(QueryType type);

        explicit VulkanQueryPool(VkDevice device, QueryType type, u32 size) noexcept;
        ~VulkanQueryPool();

        bool create() override;
        void destroy() override;

    private:

        VkDevice m_device;
        VkQueryPool m_pool;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
