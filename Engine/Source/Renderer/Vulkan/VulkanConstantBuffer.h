#include "Common.h"

#ifdef VULKAN_RENDER
#include "VulkanResource.h"
#include "VulkanMemory.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanDevice;
    class VulkanBuffer;
    class VulkanConstantBufferManager;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanUniformBuffer final class. Vulkan Render side
    */
    class VulkanConstantBuffer final
    {
    public:

        VulkanConstantBuffer() noexcept;
        explicit VulkanConstantBuffer(VulkanBuffer* buffer, u64 offset, u64 size) noexcept;
        ~VulkanConstantBuffer() = default;

        VulkanBuffer* getBuffer() const;
        u64 getOffset() const;
        u64 getSize() const;

        void set(VulkanBuffer* buffer, u64 offset, u64 size);
        bool update(u32 offset, u32 size, const void* data);

    private:

        friend VulkanConstantBufferManager;

        VulkanBuffer*   m_buffer;
        u64             m_offset;
        u64             m_size;
    };

    inline VulkanBuffer* VulkanConstantBuffer::getBuffer() const
    {
        return m_buffer;
    }

    inline u64 VulkanConstantBuffer::getOffset() const
    {
        return m_offset;
    }

    inline u64 VulkanConstantBuffer::getSize() const
    {
        return m_size;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanConstantBufferManager final class. Vulkan Render side
    */
    class VulkanConstantBufferManager final
    {
    public:

        explicit VulkanConstantBufferManager(VulkanDevice* device, VulkanResourceDeleter& resourceDeleter) noexcept;
        ~VulkanConstantBufferManager();

        //VulkanUniformBuffer
        VulkanConstantBuffer* acquireUnformBuffer(u32 requestedSize);

        void markToUse(VulkanCommandBuffer* cmdBuffer, u64 frame);
        void updateUniformBuffers();

    private:

        VulkanConstantBufferManager() = delete;
        VulkanConstantBufferManager& operator=(const VulkanConstantBufferManager&) = delete;

        struct VulkanConstantBufferPool
        {
            VulkanConstantBufferPool() noexcept = default;

            VulkanBuffer* _buffer = nullptr;
            u64 _usedSize = 0U;
            u64 _freeSize = 0U;
            u64 _poolSize = 0U;

            u32 _uniformIndex = 0;
            std::vector<VulkanConstantBuffer> _uniforms;

            VulkanConstantBuffer* prepareUniformBuffer(VulkanBuffer* buffer, u32 offset, u32 size);
            void resetPool();
        };


        VulkanConstantBufferPool* getNewPool(u64 size, u32 count);
        bool freeUniformBufferPool(VulkanConstantBufferPool* uniformPool, bool waitComplete);

        VulkanDevice& m_device;
        VulkanResourceDeleter& m_resourceDeleter;
        VulkanMemory::VulkanMemoryAllocator* m_memoryManager;

        std::deque<VulkanConstantBufferPool*> m_freePoolBuffers;
        std::deque<VulkanConstantBufferPool*> m_usedPoolBuffers;
        VulkanConstantBufferPool* m_currentPoolBuffer;

        std::map<VulkanBuffer*, VulkanConstantBufferPool*> m_uniformBuffers;

        const u64 k_bufferPoolSize;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
