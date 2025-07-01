#pragma once

#include "Renderer/Device.h"
#include "FrameProfiler.h"

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
    class VulkanBuffer;
    class VulkanSwapchain;

    class VulkanSemaphoreManager;
    class VulkanStagingBufferManager;
    class VulkanFramebufferManager;
    class VulkanRenderpassManager;
    class VulkanGraphicPipelineManager;
    class VulkanComputePipelineManager;
    class VulkanPipelineLayoutManager;
    class VulkanConstantBufferManager;
    class VulkanDescriptorSetManager;
    class VulkanSamplerManager;
    class VulkanSyncPoint;
    class VulkanStateTracker;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanCmdList : public CmdListRender
    {
    public:

        VulkanCmdList(VulkanDevice* device) noexcept;
        ~VulkanCmdList();

        void setViewport(const math::Rect& viewport, const math::float2& depth = { 0.0f, 1.0f }) override;
        void setScissor(const math::Rect& scissor) override;
        void setStencilRef(u32 mask) override;

        void beginRenderTarget(RenderTargetState& rendertarget) override;
        void endRenderTarget() override;

        void setPipelineState(GraphicsPipelineState& pipeline) override;
        void setPipelineState(ComputePipelineState& pipeline) override;

        void transition(const TextureView& textureView, TransitionOp state)override;

        void bindTexture(u32 set, u32 slot, const TextureView& textureView) override;
        void bindSampler(u32 set, u32 slot, const SamplerState& sampler) override;
        void bindUAV(u32 set, u32 slot, Buffer* buffer) override;
        void bindUAV(u32 set, u32 slot, Texture* texture) override;
        void bindConstantBuffer(u32 set, u32 slot, u32 size, const void* data) override;
        void bindPushConstant(ShaderType type, u32 size, const void* data) override;

        void bindDescriptorSet(u32 set, const std::vector<Descriptor>& descriptors) override;

        void draw(const GeometryBufferDesc& desc, u32 firstVertex, u32 vertexCount, u32 firstInstance, u32 instanceCount) override;
        void drawIndexed(const GeometryBufferDesc& desc, u32 firstIndex, u32 indexCount, u32 vertexOffest, u32 firstInstance, u32 instanceCount) override;

        void clear(Texture* texture, const color::Color& color) override;
        void clear(Texture* texture, f32 depth, u32 stencil) override;
        void clear(Buffer* buffer, u32 value) override;

        bool uploadData(Texture2D* texture, u32 size, const void* data) override;
        bool uploadData(Texture3D* texture, u32 size, const void* data) override;
        bool uploadData(Buffer* buffer, u32 offset, u32 size, const void* data) override;

        void copy(Texture* src, Texture* dst, const math::Dimension3D& size) override;
        void copy(const TextureView& src, const TextureView& dst, const math::Dimension3D& size) override;
        void copy(Buffer* src, u32 srcOffset, Buffer* dst, u32 dstOffset, u32 size) override;

        void insertDebugMarker(const std::string& marker, const color::Color& color) override;
        void beginDebugMarker(const std::string& marker, const color::Color& color) override;
        void endDebugMarker(const std::string& marker) override;

    public:

        VulkanCommandBuffer* acquireAndStartCommandBuffer(CommandTargetType type);

    public:

        Device::DeviceMask getDeviceMask() const;

        bool prepareDraw(VulkanCommandBuffer* drawBuffer);
        bool prepareDescriptorSets(VulkanCommandBuffer* drawBuffer);

        void postSubmit();

    private:

        friend VulkanDevice;

        VulkanDevice&                   m_device;
        Device::DeviceMask              m_queueMask;
        u32                             m_queueIndex;
        std::thread::id                 m_threadID;
        u32                             m_concurrencySlot;
                                        
        VulkanCommandBuffer*            m_currentCmdBuffer[toEnumType(CommandTargetType::Count)];
        VulkanConstantBufferManager*    m_CBOManager;
        VulkanDescriptorSetManager*     m_descriptorSetManager;

        VulkanRenderState               m_pendingRenderState;
        VulkanRenderState               m_currentRenderState;

        std::vector<VulkanSyncPoint*>   m_syncPoints;

    public:
        std::vector<VulkanSemaphore*> m_presentedSwapchainSemaphores; //TODO
    };

    inline Device::DeviceMask VulkanCmdList::getDeviceMask() const
    {
        return m_queueMask;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanSyncPoint : public SyncPoint
    {
    public:

        VulkanSyncPoint() = default;
        ~VulkanSyncPoint() = default;

    public:

        std::vector<VulkanSemaphore*> m_waitSubmitSemaphores;
        std::vector<VulkanSemaphore*> m_signalSubmitSemaphores;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    struct DeviceInfo
    {
        VkInstance                              _instance       = VK_NULL_HANDLE;
        VkPhysicalDevice                        _physicalDevice = VK_NULL_HANDLE;
        VkDevice                                _device         = VK_NULL_HANDLE;
        VkQueueFlags                            _queueMask      = 0;
        std::vector<std::tuple<VkQueue, u32>>   _queues;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanDevice final class. Vulkan Render side
    */
    class VulkanDevice final : public Device
    {
    public:

        static bool isDynamicStateSupported(VkDynamicState state);
        static const std::vector<VkDynamicState>& getDynamicStates();

        explicit VulkanDevice(DeviceMaskFlags mask) noexcept;
        ~VulkanDevice();

        const DeviceCaps& getDeviceCaps() const override;

        void submit(CmdList* cmd, bool wait = false) override;
        void submit(CmdList* cmd, SyncPoint* sync, bool wait = false) override;
        void waitGPUCompletion(CmdList* cmd) override;

        [[nodiscard]] Swapchain* createSwapchain(platform::Window* window, const Swapchain::SwapchainParams& params) override;
        void destroySwapchain(Swapchain* swapchain) override;

        [[nodiscard]] virtual SyncPoint* createSyncPoint(CmdList* cmd) override;
        void destroySyncPoint(CmdList* cmd, SyncPoint* sync) override;

        [[nodiscard]] TextureHandle createTexture(TextureTarget target, Format format, const math::Dimension3D& dimension, u32 layers, u32 mipmapLevel, TextureUsageFlags flags, const std::string& name = "") override;
        [[nodiscard]] TextureHandle createTexture(TextureTarget target, Format format, const math::Dimension3D& dimension, u32 layers, TextureSamples samples, TextureUsageFlags flags, const std::string& name = "") override;
        void destroyTexture(TextureHandle texture) override;

        [[nodiscard]] BufferHandle createBuffer(RenderBuffer::Type type, u16 usageFlag, u64 size, const std::string& name = "") override;
        void destroyBuffer(BufferHandle buffer) override;

    public:

        bool initialize() override;
        void destroy() override;

        void destroyFramebuffer(Framebuffer* framebuffer) override;
        void destroyRenderpass(RenderPass* renderpass) override;
        void destroyPipeline(RenderPipeline* pipeline) override;
        void destroySampler(Sampler* sampler) override;

        VkQueue getQueueByMask(DeviceMask mask);
        u32 getQueueFamilyIndexByMask(DeviceMask mask);

        const DeviceInfo& getDeviceInfo() const;
        const VulkanDeviceCaps& getVulkanDeviceCaps() const;

        VulkanStagingBufferManager* getStaginBufferManager() const;
        VulkanPipelineLayoutManager* getPipelineLayoutManager() const;
        VulkanRenderpassManager* getRenderpassManager() const;

    private:

        friend VulkanCmdList;
        friend VulkanSwapchain;

        VulkanDevice() = delete;
        VulkanDevice(const VulkanDevice&) = delete;

        template<class T>
        friend void memory::internal_delete(T* ptr, v3d::memory::MemoryLabel label, const v3d::c8* file, v3d::u32 line);

        CmdList* createCommandList_Impl(DeviceMask queueType) override;
        void destroyCommandList(CmdList* cmdList) override;

        bool createInstance();
        bool createDevice();

        const std::string                       s_vulkanApplicationName = "VulkanRender";
        static std::vector<VkDynamicState>      s_requiredDynamicStates;
        static std::vector<VkDynamicState>      s_supportedDynamicStates;

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
        VulkanPipelineLayoutManager*            m_pipelineLayoutManager;
        VulkanGraphicPipelineManager*           m_graphicPipelineManager;
        VulkanComputePipelineManager*           m_computePipelineManager;
        VulkanSamplerManager*                   m_samplerManager;

        VulkanResourceDeleter                   m_resourceDeleter;

        struct Concurrency
        {
            std::thread::id             m_threadID;
            VulkanCommandBufferManager* m_cmdBufferManager = nullptr;
        };
        std::recursive_mutex                    m_mutex;
        VulkanCommandBufferManager*             m_internalCmdBufferManager;

        std::vector<Concurrency>                m_threadedPools;
        u16                                     m_maskOfActiveThreadPool;

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
            if ((m_maskOfActiveThreadPool >> i) == 0)
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

    inline VulkanPipelineLayoutManager* VulkanDevice::getPipelineLayoutManager() const
    {
        ASSERT(m_pipelineLayoutManager, "nullptr");
        return m_pipelineLayoutManager;
    }

    inline VulkanRenderpassManager* VulkanDevice::getRenderpassManager() const
    {
        ASSERT(m_renderpassManager, "nullptr");
        return m_renderpassManager;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
