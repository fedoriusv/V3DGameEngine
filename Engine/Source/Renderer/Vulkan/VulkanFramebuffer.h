#pragma once

#include "Framebuffer.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanFramebuffer final : public Framebuffer
    {
    public:

        VulkanFramebuffer();
        ~VulkanFramebuffer();

        VkFramebuffer getHandle() const;

        bool create() override;
        void destroy() override;

    private:

        VkFramebuffer m_framebuffer;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
