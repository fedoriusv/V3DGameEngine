#pragma once

#include "Renderer/Context.h"
#include "Utils/Observable.h"
#include "Utils/Semaphore.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"
#include "VulkanDeviceCaps.h"
#include "VulkanCommandBufferManager.h"
#include "VulkanMemory.h"

#include "VulkanTransitionState.h"

#define THREADED_PRESENT 0

namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

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
        VkQueueFlags        _mask;
        VkDevice            _device;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VulkanGraphicContext final class. Vulkan Render side
    */
    class VulkanGraphicContext final : public Context,  public utils::Observer
    {
    public:

        VulkanGraphicContext() = delete;
        VulkanGraphicContext(const VulkanGraphicContext&) = delete;

        explicit VulkanGraphicContext(const platform::Window* window, DeviceMask mask) noexcept;
        ~VulkanGraphicContext();

        void beginFrame() override;
        void endFrame() override;
        void presentFrame() override;
        void submit(bool wait = false) override;

        void draw(const StreamBufferDescription& desc, u32 firstVertex, u32 vertexCount, u32 firstInstance, u32 instanceCount) override;
        void drawIndexed(const StreamBufferDescription& desc, u32 firstIndex, u32 indexCount, u32 firstInstance, u32 instanceCount) override;

        void dispatchCompute(const core::Dimension3D& groups) override;

        void bindImage(const Shader* shader, u32 bindIndex, const Image* image, s32 layer = k_generalLayer, s32 mip = k_allMipmapsLevels) override;
        void bindSampler(const Shader* shader, u32 bindIndex, const Sampler::SamplerInfo* samplerInfo) override;
        void bindSampledImage(const Shader* shader, u32 bindIndex, const Image* image, const Sampler::SamplerInfo* samplerInfo, s32 layer = k_generalLayer, s32 mip = k_allMipmapsLevels) override;
        void bindUniformsBuffer(const Shader* shader, u32 bindIndex, u32 offset, u32 size, const void* data) override;
        void bindStorageImage(const Shader* shader, u32 bindIndex, const Image* image, s32 layer = k_generalLayer, s32 mip = k_allMipmapsLevels) override;

        void transitionImages(std::vector<const Image*>& images, TransitionOp transition, s32 layer = k_generalLayer, s32 mip = k_allMipmapsLevels) override;

        void setViewport(const core::Rect32& viewport, const core::Vector2D& depth = { 0.0f, 1.0f }) override;
        void setScissor(const core::Rect32& scissor) override;

        void setRenderTarget(const RenderPass::RenderPassInfo* renderpassInfo, const Framebuffer::FramebufferInfo* framebufferInfo) override;
        void removeFramebuffer(Framebuffer* framebuffer) override;
        void removeRenderPass(RenderPass* renderpass) override;
        void invalidateRenderPass() override;

        void setPipeline(const Pipeline::PipelineGraphicInfo* pipelineInfo) override;
        void setPipeline(const Pipeline::PipelineComputeInfo* pipelineInfo) override;
        void removePipeline(Pipeline* pipeline) override;

        Image* createImage(TextureTarget target, Format format, const core::Dimension3D& dimension, u32 layers, u32 mipmapLevel, TextureUsageFlags flags, [[maybe_unused]] const std::string& name = "") override;
        Image* createImage(TextureTarget target, Format format, const core::Dimension3D& dimension, u32 layers, TextureSamples samples, TextureUsageFlags flags, [[maybe_unused]] const std::string& name = "") override;
        void removeImage(Image* image) override;
        void removeSampler(Sampler* sampler) override;

        Buffer* createBuffer(Buffer::BufferType type, u16 usageFlag, u64 size, [[maybe_unused]] const std::string& name = "") override;
        void removeBuffer(Buffer* buffer) override;

        static const std::vector<VkDynamicState>& getDynamicStates();
        static bool isDynamicState(VkDynamicState state);

        const DeviceCaps* getDeviceCaps() const override;

        void handleNotify(const utils::Observable* obj) override;

        VulkanStagingBufferManager* getStagingManager();
        VulkanCommandBuffer* getOrCreateAndStartCommandBuffer(CommandTargetType type);

        VulkanSwapchain* getSwapchain() const;

        void generateMipmaps(Image* image, u32 layer, TransitionOp state) override;

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
            CurrentCommandBufferState() noexcept;
            VulkanCommandBuffer* acquireAndStartCommandBuffer(CommandTargetType type);

            void invalidateCommandBuffer(CommandTargetType type);

            VulkanCommandBuffer* getAcitveBuffer(CommandTargetType type);
            bool isCurrentBufferAcitve(CommandTargetType type) const;

            VulkanCommandBuffer* _currentCmdBuffer[CommandTargetType::CommandTarget_Count];
            VulkanCommandBufferManager* _commandBufferMgr;
        };

        struct PendingState
        {
            bool setPendingPipeline(Pipeline* pipeline)
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

            bool isGraphicPipeline()
            {
                return m_pipeline != nullptr && m_pipeline->getType() == Pipeline::PipelineType::PipelineType_Graphic;
            }

            template<class Type>
            Type* takePipeline()
            {
                ASSERT(m_pipeline, "nullptr");
                static_assert(std::is_same<Type, VulkanGraphicPipeline>() || std::is_same<Type, VulkanComputePipeline>(), "wrong type");
                return static_cast<Type*>(std::exchange(m_pipeline, nullptr));
            }

        private:

            Pipeline* m_pipeline;
        };
        

        PendingState                m_pendingState;
        CurrentCommandBufferState   m_currentBufferState;
        VulkanContextState*         m_currentContextState;
        VulkanTransitionState       m_currentTransitionState;

        VulkanResourceDeleter       m_resourceDeleter;

        static std::vector<VkDynamicState>  s_dynamicStates;

        bool prepareDraw(VulkanCommandBuffer* drawBuffer);
        bool prepareDispatch(VulkanCommandBuffer* drawBuffer);

        void finalizeCommandBufferSubmit();

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
