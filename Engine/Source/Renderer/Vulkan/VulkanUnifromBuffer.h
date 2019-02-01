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

    class VulkanUniformBuffer final
    {
    public:

        VulkanUniformBuffer(VulkanBuffer* buffer, u64 offset, u64 size) noexcept;

        VulkanBuffer*   _buffer;
        u32             _offset;
        u32             _size;

        bool update(u32 offset, u32 size, const void* data);

    private:

    };

    class VulkanUniformBufferManager final
    {
    public:

        VulkanUniformBufferManager(VkDevice device);
        ~VulkanUniformBufferManager();

        VulkanUniformBuffer* acquireUnformBuffer(u32 requestedSize);
        VulkanUniformBuffer* findUniformBuffer(const VulkanBuffer* buffer, u32 requestedSize);

    private:

        struct VulkanUniformBufferPool
        {
            VulkanBuffer* _buffer;
            u64 _usedSize;
            u64 _freeSize;
        };

        VulkanUniformBufferPool* getFreePool();

        std::deque<VulkanUniformBufferPool*> m_freeBuffers;
        std::deque<VulkanUniformBufferPool*> m_usedUBuffers;
        VulkanUniformBufferPool* m_currentBuffer;

        std::map<VulkanBuffer*, VulkanUniformBufferPool*> m_uniformBuffers;

        VkDevice m_device;
        VulkanMemory::VulkanMemoryAllocator* m_memoryManager;

        const u64 k_bufferPoolSize = 1 * 1024 * 1024;
    };

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
