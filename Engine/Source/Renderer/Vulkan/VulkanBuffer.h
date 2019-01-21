#pragma once

#include "Common.h"
#include "Renderer/Buffer.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"
#include "VulkanMemory.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * VulkanBuffer class. Render side
    */
    class VulkanBuffer final : public Buffer
    {
    public:

        VulkanBuffer(VulkanMemory* memory, VkDevice device, u16 usageFlag);
        ~VulkanBuffer();

        bool create() override;
        void destroy() override;

    private:

        VkDevice m_device;
        VulkanMemory* m_memoryManager;

        u32 m_usageFlags;

        static VulkanMemory::VulkanMemoryAllocator* s_memoryAllocator;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
