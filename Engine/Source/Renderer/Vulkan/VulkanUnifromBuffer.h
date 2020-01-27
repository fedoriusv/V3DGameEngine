#include "Common.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"
#include "VulkanResource.h"
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

        VulkanUniformBuffer() = delete;

        VulkanUniformBuffer(VulkanBuffer* buffer, u64 offset, u64 size) noexcept;
        ~VulkanUniformBuffer();

        VulkanBuffer* getBuffer() const;
        u64 getOffset() const;
        u64 getSize() const;

        bool update(u32 offset, u32 size, const void* data);

    private:

        VulkanBuffer*   m_buffer;
        u64             m_offset;
        u64             m_size;
    };

    class VulkanUniformBufferManager final
    {
    public:

        VulkanUniformBufferManager() = delete;
        VulkanUniformBufferManager(const VulkanUniformBufferManager&) = delete;
        VulkanUniformBufferManager& operator=(const VulkanUniformBufferManager&) = delete;

        explicit VulkanUniformBufferManager(VkDevice device, VulkanResourceDeleter& resourceDeleter) noexcept;
        ~VulkanUniformBufferManager();

        VulkanUniformBuffer* acquireUnformBuffer(u32 requestedSize);
        void updateUniformBuffers();

    private:

        struct VulkanUniformBufferPool
        {
            VulkanBuffer* _buffer;
            u64 _usedSize;
            u64 _freeSize;
            u64 _poolSize;

            std::vector<VulkanUniformBuffer*> _uniformList;

            void resetPool();
            void addUniformBuffer(VulkanUniformBuffer* uniformBuffer, u64 size);
        };

        bool freeUniformBufferPool(VulkanUniformBufferPool* uniformPool, bool waitComplete);

        VulkanUniformBufferPool* getNewPool(u64 size);

        VkDevice m_device;

        std::deque<VulkanUniformBufferPool*> m_freePoolBuffers;
        std::deque<VulkanUniformBufferPool*> m_usedPoolBuffers;
        VulkanUniformBufferPool* m_currentPoolBuffer;

        std::map<VulkanBuffer*, VulkanUniformBufferPool*> m_uniformBuffers;

        VulkanMemory::VulkanMemoryAllocator* m_memoryManager; 
        VulkanResourceDeleter& m_resourceDeleter;

        const u64 k_bufferPoolSize;
    };

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
