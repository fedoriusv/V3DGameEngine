#pragma once

#include "RenderPass.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanRenderPass final : public RenderPass
    {
    public:

        VulkanRenderPass();
        ~VulkanRenderPass();

        VkRenderPass getHandle() const;

        bool create() override;
        void destroy() override;

    private:

        VkRenderPass m_renderpass;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
