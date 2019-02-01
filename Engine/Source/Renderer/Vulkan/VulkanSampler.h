#pragma once

#include "Common.h"
#include "Renderer/Sampler.h"

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
    * VulkanSampler class. Vulkan Render side
    */
    class VulkanSampler final : public Sampler, public VulkanResource
    {
    public:

        VulkanSampler();
        ~VulkanSampler();

        bool create() override;
        void destroy() override;

        VkSampler getHandle() const;

    private:

        VkDevice  m_device;
        VkSampler m_sampler;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
