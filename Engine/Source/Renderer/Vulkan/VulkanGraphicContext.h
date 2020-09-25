#pragma once

#include "Renderer/Context.h"
#include "Utils/Observable.h"
#include "Utils/Semaphore.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"
#include "VulkanDeviceCaps.h"
#include "VulkanCommandBufferManager.h"
#include "VulkanMemory.h"

#define THREADED_PRESENT 0

namespace v3d
{
namespace renderer
{
namespace vk
{
    class VulkanImage;
    class VulkanSampler;
    class VulkanRenderPass;
    class VulkanFramebuffer;
    class VulkanGraphicPipeline;

    class VulkanContextState;
    class VulkanSwapchain;

    class VulkanPipelineLayoutManager;
    class VulkanDescriptorSetManager;
    class VulkanStagingBufferManager;
    class VulkanUniformBufferManager;
    class VulkanSamplerManager;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    struct DeviceInfo
    {
        VkInstance          _instance;
        VkPhysicalDevice    _physicalDevice;
        u32                 _queueFamilyIndex;
        VkDevice            _device;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * VulkanGraphicContext final class. Vulkan Render side
    */
    class VulkanGraphicContext final : public Context,  public utils::Observer
    {
    public:

        explicit VulkanGraphicContext(const platform::Window* window) noexcept;
        ~VulkanGraphicContext();

        void beginFrame() override;
        void endFrame() override;
        void presentFrame() override;
        void submit(bool wait = false) override;

        void draw(const StreamBufferDescription& desc, u32 firstVertex, u32 vertexCount, u32 firstInstance, u32 instanceCount) override;
        void drawIndexed(const StreamBufferDescription& desc, u32 firstIndex, u32 indexCount, u32 firstInstance, u32 instanceCount) override;

        void bindImage(const Shader* shader, u32 bindIndex, const Image* image) override;
        void bindSampler(const Shader* shader, u32 bindIndex, const Sampler::SamplerInfo* samplerInfo) override;
        void bindSampledImage(const Shader* shader, u32 bindIndex, const Image* image, const Sampler::SamplerInfo* samplerInfo) override;
        void bindUniformsBuffer(const Shader* shader, u32 bindIndex, u32 offset, u32 size, const void* data) override;

        void transitionImages(const std::vector<Image*>& images, TransitionOp transition, s32 layer = -1) override;

        void setViewport(const core::Rect32& viewport, const core::Vector2D& depth = { 0.0f, 1.0f }) override;
        void setScissor(const core::Rect32& scissor) override;

        void setRenderTarget(const RenderPass::RenderPassInfo* renderpassInfo, const Framebuffer::FramebufferInfo* framebufferInfo) override;
        void removeFramebuffer(Framebuffer* framebuffer) override;
        void removeRenderPass(RenderPass* renderpass) override;
        void invalidateRenderPass() override;

        void setPipeline(const Pipeline::PipelineGraphicInfo* pipelineInfo) override;
        void removePipeline(Pipeline* pipeline) override;

        Image* createImage(TextureTarget target, renderer::Format format, const core::Dimension3D& dimension, u32 layers, u32 mipmapLevel, TextureUsageFlags flags, [[maybe_unused]] const std::string& name = "") override;
        Image* createImage(renderer::Format format, const core::Dimension3D& dimension, TextureSamples samples, TextureUsageFlags flags, [[maybe_unused]] const std::string& name = "") override;
        void removeImage(Image* image) override;

        Buffer* createBuffer(Buffer::BufferType type, u16 usageFlag, u64 size, [[maybe_unused]] const std::string& name = "") override;
        void removeBuffer(Buffer* buffer) override;

        void removeSampler(Sampler* sampler) override;

        static const std::vector<VkDynamicState>& getDynamicStates();
        static bool isDynamicState(VkDynamicState state);

        const DeviceCaps* getDeviceCaps() const override;

        void handleNotify(utils::Observable* obj) override;

        VulkanStagingBufferManager* getStagingManager();
        VulkanCommandBuffer* getOrCreateAndStartCommandBuffer(CommandTargetType type);

        VulkanSwapchain* getSwapchain() const;

    private:

        VulkanDeviceCaps&   m_deviceCaps;
        DeviceInfo          m_deviceInfo;

        const std::string s_vulkanApplicationName = "VulkanGraphicContext";

        bool initialize() override;
        void destroy() override;

        void clearBackbuffer(const core::Vector4D & color) override;

        Framebuffer* createFramebuffer(const std::vector<Image*>& images, const core::Dimension2D& size) override;
        RenderPass* createRenderPass(const RenderPassDescription* renderpassDesc) override;
        Pipeline* createPipeline(Pipeline::PipelineType type) override;
        Sampler* createSampler(const SamplerDescription& desc) override;

        void invalidateStates();

        bool createInstance();
        bool createDevice();

        std::vector<VkQueue>        m_queueList;
        class VulkanSwapchain*      m_swapchain;

        VulkanCommandBufferManager* m_cmdBufferManager;
        VulkanPipelineLayoutManager* m_pipelineLayoutManager;
        VulkanDescriptorSetManager* m_descriptorSetManager;
        VulkanStagingBufferManager* m_stagingBufferManager;
        VulkanUniformBufferManager* m_uniformBufferManager;

        VulkanMemory::VulkanMemoryAllocator* m_imageMemoryManager;
        VulkanMemory::VulkanMemoryAllocator* m_bufferMemoryManager;

        RenderPassManager*          m_renderpassManager;
        FramebufferManager*         m_framebufferManager;
        PipelineManager*            m_pipelineManager;
        SamplerManager*             m_samplerManager;

        struct CurrentCommandBufferState
        {
            CurrentCommandBufferState();

            void invalidateCommandBuffer(CommandTargetType type);

            VulkanCommandBuffer* getAcitveBuffer(CommandTargetType type);
            bool isCurrentBufferAcitve(CommandTargetType type) const;

            VulkanCommandBuffer* _currentCmdBuffer[CommandTargetType::CommandTarget_Count];

        };

        struct PendingState
        {
            bool setPendingPipeline(VulkanGraphicPipeline* pipeline)
            {
                if (pipeline && m_pipeline != pipeline)
                {
                    m_pipeline = pipeline;
                    return true;
                }

                return false;
            }

            bool isPipeline()
            {
                return m_pipeline != nullptr;
            }

            VulkanGraphicPipeline* takePipeline()
            {
                ASSERT(m_pipeline, "nullptr");
                return std::exchange(m_pipeline, nullptr);
            }

        private:

            VulkanGraphicPipeline* m_pipeline;
        };
        

        PendingState                m_pendingState;
        CurrentCommandBufferState   m_currentBufferState;
        VulkanContextState*         m_currentContextState;

        VulkanResourceDeleter       m_resourceDeleter;

        static std::vector<VkDynamicState>  s_dynamicStates;
        bool prepareDraw(VulkanCommandBuffer* drawBuffer);

        const platform::Window* m_window;
#if THREADED_PRESENT
        class PresentThread* m_presentThread;
#endif //THREADED_PRESENT
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

#if THREADED_PRESENT
    class PresentThread
    {
    public:

        explicit PresentThread(class VulkanSwapchain* swapchain);
        ~PresentThread();

        void requestAcquireImage(u32& index);
        void requestPresent(VkQueue queue, VkSemaphore semaphore);

    private:

        void internalPresent();
        void internalAcquire();

        static void presentLoop(void* data);

        std::thread m_thread;
        std::atomic_bool m_run;

        utils::Semaphore m_wakeupSemaphore;
        utils::Semaphore m_waitSemaphore;

        class VulkanSwapchain* m_swapchain;
        VkQueue m_queue;
        VkSemaphore m_semaphore;

        u32 m_index;
    };
#endif //THREADED_PRESENT

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
