#pragma once

#include "Renderer/Core/Context.h"
#include "Renderer/Core/RenderFrameProfiler.h"
#include "Utils/Observable.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"
#include "VulkanDeviceCaps.h"
#include "VulkanMemory.h"
#include "VulkanTransitionState.h"
#include "VulkanCommandBufferManager.h"

#define THREADED_PRESENT 0

namespace v3d
{
namespace renderer
{
namespace vk
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class VulkanGraphicPipeline;
    class VulkanSemaphore;
    class VulkanContextState;
    class VulkanSwapchain;

    class VulkanPipelineLayoutManager;
    class VulkanDescriptorSetManager;
    class VulkanStagingBufferManager;
    class VulkanUniformBufferManager;
    class VulkanSemaphoreManager;
    class VulkanQueryPoolManager;

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
    * @brief VulkanContext final class. Vulkan Render side
    */
    class VulkanContext final : public Context, public utils::Observer
    {
    public:

        explicit VulkanContext(platform::Window* window, DeviceMask mask) noexcept;
        ~VulkanContext();

        const DeviceCaps* getDeviceCaps() const override;

        void beginFrame() override;
        void endFrame() override;
        void presentFrame() override;
        void submit(bool wait = false) override;

        void beginQuery(const Query* query, u32 id = 0, const std::string& tag = "") override;
        void endQuery(const  Query* query, u32 id = 0, const std::string& tag = "") override;
        void timestampQuery(const Query* query, u32 id = 0, const std::string& tag = "") override;

        void draw(const StreamBufferDescription& desc, u32 firstVertex, u32 vertexCount, u32 firstInstance, u32 instanceCount) override;
        void drawIndexed(const StreamBufferDescription& desc, u32 firstIndex, u32 indexCount, u32 firstInstance, u32 instanceCount) override;

        void dispatchCompute(const core::Dimension3D& groups) override;

        void bindImage(const Shader* shader, u32 bindIndex, const Image* image, s32 layer = k_generalLayer, s32 mip = k_allMipmapsLevels) override;
        void bindSampler(const Shader* shader, u32 bindIndex, const Sampler::SamplerInfo* samplerInfo) override;
        void bindSampledImage(const Shader* shader, u32 bindIndex, const Image* image, const Sampler::SamplerInfo* samplerInfo, s32 layer = k_generalLayer, s32 mip = k_allMipmapsLevels) override;
        void bindUniformsBuffer(const Shader* shader, u32 bindIndex, u32 offset, u32 size, const void* data) override;
        void bindStorageImage(const Shader* shader, u32 bindIndex, const Image* image, s32 layer = k_generalLayer, s32 mip = k_allMipmapsLevels) override;

        void transitionImages(std::vector<std::tuple<const Image*, Image::Subresource>>& images, TransitionOp transition) override;

        void setViewport(const core::Rect32& viewport, const core::Vector2D& depth = { 0.0f, 1.0f }) override;
        void setScissor(const core::Rect32& scissor) override;

        void setRenderTarget(const RenderPass::RenderPassInfo* renderpassInfo, const Framebuffer::FramebufferInfo* framebufferInfo) override;
        void invalidateRenderTarget() override;

        void setPipeline(const Pipeline::PipelineGraphicInfo* pipelineInfo) override;
        void setPipeline(const Pipeline::PipelineComputeInfo* pipelineInfo) override;

        Image* createImage(TextureTarget target, Format format, const core::Dimension3D& dimension, u32 layers, u32 mipmapLevel, TextureUsageFlags flags, [[maybe_unused]] const std::string& name = "") override;
        Image* createImage(TextureTarget target, Format format, const core::Dimension3D& dimension, u32 layers, TextureSamples samples, TextureUsageFlags flags, [[maybe_unused]] const std::string& name = "") override;
        void removeImage(Image* image) override;

        Buffer* createBuffer(Buffer::BufferType type, u16 usageFlag, u64 size, [[maybe_unused]] const std::string& name = "") override;
        void removeBuffer(Buffer* buffer) override;

        Sampler* createSampler(const SamplerDescription& desc) override;
        void removeSampler(Sampler* sampler) override;

        Pipeline* createPipeline(Pipeline::PipelineType type, [[maybe_unused]] const std::string& name = "") override;
        void removePipeline(Pipeline* pipeline) override;

        Framebuffer* createFramebuffer(const std::vector<Image*>& images, const core::Dimension2D& size) override;
        void removeFramebuffer(Framebuffer* framebuffer) override;

        RenderPass* createRenderPass(const RenderPassDescription* renderpassDesc) override;
        void removeRenderPass(RenderPass* renderpass) override;

        Query* createQuery(QueryType type, u32 size, const Query::QueryRespose& callback, [[maybe_unused]] const std::string& name = "") override;
        void removeQuery(Query* query) override;

        static const std::vector<VkDynamicState>& getDynamicStates();
        static bool isDynamicState(VkDynamicState state);

        void handleNotify(const utils::Observable* obj) override;

        VulkanStagingBufferManager* getStagingManager();
        VulkanCommandBuffer* getOrCreateAndStartCommandBuffer(CommandTargetType type);

        VulkanSwapchain* getSwapchain() const;

        void clearBackbuffer(const core::Vector4D & color) override;
        void generateMipmaps(Image* image, u32 layer, TransitionOp state) override;

    private:

        VulkanContext() = delete;
        VulkanContext(const VulkanContext&) = delete;

        const std::string s_vulkanApplicationName = "VulkanContext";

        bool initialize() override;
        void destroy() override;

        bool prepareDraw(VulkanCommandBuffer* drawBuffer);
        bool prepareDispatch(VulkanCommandBuffer* drawBuffer);

        void invalidateStates();
        void finalizeSubmit();

        bool createInstance();
        bool createDevice();

        VulkanDeviceCaps& m_deviceCaps;
        DeviceInfo m_deviceInfo;

        std::vector<VkQueue>        m_queueList;
        class VulkanSwapchain*      m_swapchain;

        VulkanCommandBufferManager* m_cmdBufferManager;
        VulkanPipelineLayoutManager* m_pipelineLayoutManager;
        VulkanDescriptorSetManager* m_descriptorSetManager;
        VulkanStagingBufferManager* m_stagingBufferManager;
        VulkanUniformBufferManager* m_uniformBufferManager;
        VulkanSemaphoreManager* m_semaphoreManager;
        VulkanQueryPoolManager* m_renderQueryPoolManager;

        RenderPassManager* m_renderpassManager;
        FramebufferManager* m_framebufferManager;
        PipelineManager* m_pipelineManager;
        SamplerManager* m_samplerManager;

        VulkanMemory::VulkanMemoryAllocator* m_imageMemoryManager;
        VulkanMemory::VulkanMemoryAllocator* m_bufferMemoryManager;

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
                if (pipeline && _pipeline != pipeline)
                {
                    _pipeline = pipeline;
                    return true;
                }

                return false;
            }

            bool isPipeline()
            {
                return _pipeline != nullptr;
            }

            bool isGraphicPipeline()
            {
                return _pipeline != nullptr && _pipeline->getType() == Pipeline::PipelineType::PipelineType_Graphic;
            }

            bool isComputePipeline()
            {
                return _pipeline != nullptr && _pipeline->getType() == Pipeline::PipelineType::PipelineType_Compute;
            }

            template<class Type>
            Type* takePipeline()
            {
                ASSERT(_pipeline, "nullptr");
                static_assert(std::is_same<Type, VulkanGraphicPipeline>() || std::is_same<Type, VulkanComputePipeline>(), "wrong type");
                return static_cast<Type*>(std::exchange(_pipeline, nullptr));
            }

            std::vector<VkDescriptorSet> _descriptorSets;
            std::vector<u32> _descriptorOffsets;

            std::vector<VulkanSemaphore*> _presentSemaphores;

        private:

            Pipeline* _pipeline;
        };

        PendingState                m_pendingState;
        CurrentCommandBufferState   m_currentBufferState;
        VulkanContextState*         m_currentContextState;
        VulkanTransitionState       m_currentTransitionState;

        std::vector<VulkanSemaphore*> m_waitSemaphores;
        std::vector<VulkanSemaphore*> m_submitSemaphores;
        std::vector<VulkanSemaphore*> m_presentSemaphores;

        VulkanResourceDeleter m_resourceDeleter;

        bool m_insideFrame;
        platform::Window* const m_window;

#if FRAME_PROFILER_ENABLE
        RenderFrameProfiler* m_CPUProfiler;
#endif //FRAME_PROFILER_ENABLE

#if THREADED_PRESENT
        class PresentThread* m_presentThread;
#endif //THREADED_PRESENT

        static std::vector<VkDynamicState> s_dynamicStates;
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
