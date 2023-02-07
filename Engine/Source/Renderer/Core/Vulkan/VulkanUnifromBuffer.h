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
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanBuffer;
    class VulkanUniformBufferManager;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanUniformBuffer final class. Vulkan Render side
    */
    class VulkanUniformBuffer final
    {
    public:

        VulkanUniformBuffer() noexcept;
        VulkanUniformBuffer(VulkanBuffer* buffer, u64 offset, u64 size) noexcept;
        ~VulkanUniformBuffer() = default;

        VulkanBuffer* getBuffer() const;
        u64 getOffset() const;
        u64 getSize() const;

        bool update(u32 offset, u32 size, const void* data);

    private:

        friend VulkanUniformBufferManager;
        void set(VulkanBuffer* buffer, u64 offset, u64 size);

        VulkanBuffer*   m_buffer;
        u64             m_offset;
        u64             m_size;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanUniformBufferManager final class. Vulkan Render side
    */
    class VulkanUniformBufferManager final
    {
    public:

        explicit VulkanUniformBufferManager(VkDevice device, VulkanResourceDeleter& resourceDeleter) noexcept;
        ~VulkanUniformBufferManager();

        //VulkanUniformBuffer
        VulkanUniformBuffer* acquireUnformBuffer(u32 requestedSize);

        void markToUse(VulkanCommandBuffer* cmdBuffer, u64 frame);
        void updateUniformBuffers();

    private:

        VulkanUniformBufferManager() = delete;
        VulkanUniformBufferManager(const VulkanUniformBufferManager&) = delete;
        VulkanUniformBufferManager& operator=(const VulkanUniformBufferManager&) = delete;

        struct VulkanUniformBufferPool
        {
            VulkanUniformBufferPool() noexcept = default;

            VulkanBuffer* _buffer = nullptr;
            u64 _usedSize = 0U;
            u64 _freeSize = 0U;
            u64 _poolSize = 0U;

            u32 _uniformIndex = 0;
            std::vector<VulkanUniformBuffer> _uniforms;

            VulkanUniformBuffer* prepareUniformBuffer(VulkanBuffer* buffer, u32 offset, u32 size);
            void resetPool();
        };


        VulkanUniformBufferPool* getNewPool(u64 size, u32 count);
        bool freeUniformBufferPool(VulkanUniformBufferPool* uniformPool, bool waitComplete);

        VkDevice m_device;

        std::deque<VulkanUniformBufferPool*> m_freePoolBuffers;
        std::deque<VulkanUniformBufferPool*> m_usedPoolBuffers;
        VulkanUniformBufferPool* m_currentPoolBuffer;

        std::map<VulkanBuffer*, VulkanUniformBufferPool*> m_uniformBuffers;

        VulkanMemory::VulkanMemoryAllocator* m_memoryManager; 
        VulkanResourceDeleter& m_resourceDeleter;

        const u64 k_bufferPoolSize;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
