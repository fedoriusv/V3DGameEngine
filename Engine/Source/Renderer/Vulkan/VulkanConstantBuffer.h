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

    struct ConstantBufferRange
    {
        VulkanBuffer*         _buffer;
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

        ConstantBufferRange acquireConstantBuffer(u32 requestedSize);

        void markToUse(VulkanCommandBuffer* cmdBuffer, u64 frame);
        void updateStatus();

        static bool update(VulkanBuffer* buffer, u32 offset, u32 size, const void* data);

    private:

        VulkanConstantBufferManager() = delete;
        VulkanConstantBufferManager& operator=(const VulkanConstantBufferManager&) = delete;

        VulkanDevice&                           m_device;
        VulkanMemory::VulkanMemoryAllocator*    m_memoryManager;

        std::deque<VulkanBuffer*>       m_freeConstantBuffers;
        std::vector<VulkanBuffer*>      m_usedConstantBuffers;
        ConstantBufferRange             m_currentConstantBuffer;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
