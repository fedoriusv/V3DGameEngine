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

        explicit VulkanConstantBuffer(VulkanDevice* device, VulkanMemory::VulkanMemoryAllocator* alloc, u64 size, const std::string& name = "ConstantBuffer") noexcept;
        ~VulkanConstantBuffer() = default;

        bool create();
        void destroy();
        bool update(u32 offset, u32 size, const void* data);

        VulkanBuffer* getBuffer() const;
        u64 getOffset() const;
        u64 getSize() const;

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

    struct ConstantBufferRange
    {
        VulkanConstantBuffer* _CB;
        u64                   _offset;
        u64                   _freeSize;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanConstantBufferManager final class. Vulkan Render side. 
    * Singlethreaded
    */
    class VulkanConstantBufferManager final
    {
    public:

        explicit VulkanConstantBufferManager(VulkanDevice* device) noexcept;
        ~VulkanConstantBufferManager();

        ConstantBufferRange acquireUnformBuffer(u32 requestedSize);

        void markToUse(VulkanCommandBuffer* cmdBuffer, u64 frame);
        void updateStatus();

    private:

        VulkanConstantBufferManager() = delete;
        VulkanConstantBufferManager& operator=(const VulkanConstantBufferManager&) = delete;

        VulkanDevice&                           m_device;
        VulkanMemory::VulkanMemoryAllocator*    m_memoryManager;

        std::deque<VulkanConstantBuffer*>       m_freeConstantBuffers;
        std::vector<VulkanConstantBuffer*>      m_usedConstantBuffers;
        ConstantBufferRange                     m_currentConstantBuffer;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
