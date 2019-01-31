#include "Common.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"
#include "VulkanMemory.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    class VulkanBuffer;

    class VulkanUnifromBuffer
    {
    public:
        VulkanBuffer* _buffer;
        u32 _offset;
        u32 _size;
    };

    class VulkanUniformBufferManager final
    {
    public:

        VulkanUniformBufferManager(VkDevice device);
        ~VulkanUniformBufferManager();

        VulkanUnifromBuffer* acquireUnformBuffer(u32 size);


    private:

        std::deque<VulkanBuffer*> m_freeBuffers;
        std::deque<VulkanBuffer*> m_usedUBuffers;
        VulkanBuffer* m_currentBuffer;

        std::list<VulkanUnifromBuffer*> m_uniformBuffers;

        VkDevice m_device;
        VulkanMemory::VulkanMemoryAllocator* m_memoryManager;
    };

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
