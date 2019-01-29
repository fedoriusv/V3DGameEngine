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

    class VulkanUnifromBuffer
    {
    public:

    private:

        VulkanBuffer* m_buffer;
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
