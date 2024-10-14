#pragma once

#include "Renderer/Device.h"

#ifdef VULKAN_RENDER
#   include "VulkanWrapper.h"
#   include "VulkanDeviceCaps.h"
#   include "VulkanResource.h"
#   include "VulkanMemory.h"
#   include "VulkanRenderState.h"
#   include "VulkanCommandBufferManager.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanDevice;
    class VulkanImage;
    class VulkanSwapchain;

    class VulkanSemaphoreManager;
    class VulkanStagingBufferManager;
    class VulkanFramebufferManager;
    class VulkanRenderpassManager;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanCmdList : public CmdListRender
    {
    public:

        VulkanCmdList(VulkanDevice* device) noexcept;
        ~VulkanCmdList();

        void setViewport(const math::Rect32& viewport, const math::Vector2D& depth = { 0.0f, 1.0f }) override;
        void setScissor(const math::Rect32& scissor) override;
        void setStencilRef(u32 mask) override;

        void beginRenderTarget(const RenderTargetState& rendertarget) override;
        void endRenderTarget() override;

        void setPipelineState(GraphicsPipelineState& pipeline) override;

        void transition(const TextureView& texture, TransitionOp state)override;

        void bindTexture(u32 binding, Texture* texture) override;
        void bindBuffer(u32 binding, Buffer* buffer) override;
        void bindSampler(u32 binding, SamplerState* sampler) override;
        void bindConstantBuffer(u32 binding, u32 size, void* data) override;

        void draw(const GeometryBufferDesc& desc, u32 firstVertex, u32 vertexCount, u32 firstInstance, u32 instanceCount) override;
        void drawIndexed(const GeometryBufferDesc& desc, u32 firstIndex, u32 indexCount, u32 firstInstance, u32 instanceCount) override;

        void clear(Texture* texture, const render::Color& color) override;
        void clear(Texture* texture, f32 depth, u32 stencil) override;

        bool uploadData(Texture2D* texture, const math::Dimension2D& offset, const math::Dimension2D& size, u32 mipLevel, const void* data) override;
        bool uploadData(Texture3D* texture, const math::Dimension3D& offset, const math::Dimension3D& size, u32 mipLevel, const void* data) override;
        bool uploadData(Buffer* buffer, u32 offset, u32 size, void* data) override;

    public:

        VulkanCommandBuffer* acquireAndStartCommandBuffer(CommandTargetType type);

    public:

        void postSubmit();
        void postPresent();


        VulkanDevice&        m_device;
        Device::DeviceMask   m_queueMask;
        u32                  m_queueIndex;
        std::thread::id      m_threadID;
        u32                  m_concurrencySlot;

        VulkanCommandBuffer* m_currentCmdBuffer[toEnumType(CommandTargetType::Count)];

        VulkanRenderState    m_pendingRenderState;
        VulkanRenderState    m_currentRenderState;

        std::vector<VulkanSemaphore*> m_waitSemaphores;
        std::vector<VulkanSemaphore*> m_submitSemaphores;
        std::vector<VulkanSemaphore*> m_presentSemaphores;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    struct DeviceInfo
    {
        VkInstance                              _instance;
        VkPhysicalDevice                        _physicalDevice;
        VkDevice                                _device;
        VkQueueFlags                            _queueMask;
        std::vector<std::tuple<VkQueue, u32>>   _queues;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanDevice final class. Vulkan Render side
    */
    class VulkanDevice final : public Device
    {
    public:

        static bool isDynamicState(VkDynamicState state);
        static const std::vector<VkDynamicState>& getDynamicStates();

        explicit VulkanDevice(DeviceMaskFlags mask) noexcept;
        ~VulkanDevice();

        const DeviceCaps& getDeviceCaps() const override;

        void submit(CmdList* cmd, bool wait = false) override;

        Swapchain* createSwapchain(platform::Window* window, const Swapchain::SwapchainParams& params) override;
        void destroySwapchain(Swapchain* swapchain) override;

    public:

        bool initialize() override;
        void destroy() override;

        VkQueue getQueueByMask(DeviceMask mask);
        u32 getQueueFamilyIndexByMask(DeviceMask mask);

        const DeviceInfo& getDeviceInfo() const;
        const VulkanDeviceCaps& getVulkanDeviceCaps() const;

        VulkanStagingBufferManager* getStaginBufferManager() const;

    private:

        friend VulkanSemaphoreManager;
        friend VulkanCommandBufferManager;
        friend VulkanFramebufferManager;
        friend VulkanRenderpassManager;
        friend VulkanImage;
        friend VulkanCmdList;

        VulkanDevice() = delete;
        VulkanDevice(const VulkanDevice&) = delete;

        template<class T>
        friend void memory::internal_delete(T* ptr, v3d::memory::MemoryLabel label, const v3d::c8* file, v3d::u32 line);

        CmdList* createCommandList_Impl(DeviceMask queueType) override;
        void destroyCommandList(CmdList* cmdList) override;

        bool createInstance();
        bool createDevice();

        const std::string                       s_vulkanApplicationName = "VulkanRender";
        static std::vector<VkDynamicState>      s_dynamicStates;

        DeviceInfo                              m_deviceInfo;
        VulkanDeviceCaps                        m_deviceCaps;

        std::vector<VulkanCmdList*>             m_cmdLists;
        std::vector<VulkanSwapchain*>           m_swapchainList;

        VulkanMemory::VulkanMemoryAllocator*    m_imageMemoryManager;
        VulkanMemory::VulkanMemoryAllocator*    m_bufferMemoryManager;
        VulkanStagingBufferManager*             m_stagingBufferManager;
        VulkanSemaphoreManager*                 m_semaphoreManager;
        VulkanFramebufferManager*               m_framebufferManager;
        VulkanRenderpassManager*                m_renderpassManager;

        VulkanResourceDeleter                   m_resourceDeleter;

        struct Concurrency
        {
            std::thread::id             m_threadID;
            VulkanCommandBufferManager* m_cmdBufferManager;
        };

        std::vector<Concurrency> m_threadedPools;
        u16 m_maskOfActiveThreadPool;

        s32 getFreeThreadSlot() const;
        u32 prepareConcurrencySlot();
    };

    inline const DeviceCaps& VulkanDevice::getDeviceCaps() const
    {
        return m_deviceCaps;
    }

    inline const VulkanDeviceCaps& VulkanDevice::getVulkanDeviceCaps() const
    {
        return *static_cast<const VulkanDeviceCaps*>(&m_deviceCaps);
    }

    inline const DeviceInfo& VulkanDevice::getDeviceInfo() const
    {
        return m_deviceInfo;
    }

    inline VkQueue VulkanDevice::getQueueByMask(DeviceMask mask)
    {
        ASSERT(mask & m_deviceInfo._queueMask, "Queue of this type is not enabled");
        return std::get<0>(m_deviceInfo._queues[mask >> 1]);
    }

    inline u32 VulkanDevice::getQueueFamilyIndexByMask(DeviceMask mask)
    {
        ASSERT(mask & m_deviceInfo._queueMask, "Queue of this type is not enabled");
        return std::get<1>(m_deviceInfo._queues[mask >> 1]);
    }

    inline s32 VulkanDevice::getFreeThreadSlot() const
    {
        for (u32 i = 0; i < m_threadedPools.size(); ++i)
        {
            if (((m_maskOfActiveThreadPool >> i) & 0b0) == 0)
            {
                return i;
            }
        }

        ASSERT(false, "All slots are occupated");
        return -1;
    }

    inline VulkanStagingBufferManager* VulkanDevice::getStaginBufferManager() const
    {
        ASSERT(m_stagingBufferManager, "nullptr");
        return m_stagingBufferManager;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
