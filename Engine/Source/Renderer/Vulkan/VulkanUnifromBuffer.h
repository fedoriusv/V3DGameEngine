#include "Common.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    class VulkanBuffer;

    struct VulkanUnifromBuffer
    {
        VulkanBuffer* _buffer;
        u32 _offset;
        u32 _size;
    };


    class VulkanUniformBufferManager
    {
    public:

        VulkanUniformBufferManager();
        VulkanUnifromBuffer* acquireUnformBuffer();

    private:

        std::vector<VulkanBuffer*> m_buffers;
    };

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
