#pragma once

#include "Renderer/Context.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"
#include "VulkanDeviceCaps.h"
#include "VulkanDescriptorSet.h"
#include "VulkanCommandBufferManager.h"
#include "VulkanMemory.h"

namespace v3d
{
namespace renderer
{
namespace vk
{
    class VulkanImage;
    class VulkanRenderPass;
    class VulkanFramebuffer;
    class VulkanGraphicPipeline;
    class VulkanContextState;
    class VulkanStaginBufferManager;
    class VulkanUniformBufferManager;

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
    class VulkanGraphicContext final : public Context
    {
    public:

        explicit VulkanGraphicContext(const platform::Window* window) noexcept;
        ~VulkanGraphicContext();

        void beginFrame() override;
        void endFrame() override;
        void presentFrame() override;
        void submit(bool wait = false) override;

        void draw(StreamBufferDescription& desc, u32 firstVertex, u32 vertexCount, u32 firstInstance, u32 instanceCount) override;
        void drawIndexed() override;

        void bindTexture(const Shader* shader, u32 bindIndex, const Image* image) override;
        void bindUniformsBuffer(const Shader* shader, u32 bindIndex, u32 offset, u32 size, const void* data) override;

        //void bindVertexBuffers(const std::vector<Buffer*>& buffer, const std::vector<u64>& offsets)  override;

        void setViewport(const core::Rect32& viewport, const core::Vector2D& depth = { 0.0f, 1.0f }) override;
        void setScissor(const core::Rect32& scissor) override;

        void setRenderTarget(const RenderPass::RenderPassInfo* renderpassInfo, const std::vector<Image*>& attachments, const RenderPass::ClearValueInfo* clearInfo, 
            const std::tuple<ObjectTracker<RenderPass>*, ObjectTracker<Framebuffer>*>& trackers) override;
        //void removeRenderTarget(const RenderPass::RenderPassInfo * renderpassInfo, const std::vector<Image*>& attachments, const RenderPass::ClearValueInfo * clearInfo) override;
        void removeFramebuffer(Framebuffer* framebuffer) override;
        void removeRenderPass(RenderPass* renderpass) override;
        void invalidateRenderPass() override;

        void setPipeline(const Pipeline::PipelineGraphicInfo* pipelineInfo, ObjectTracker<Pipeline>* tracker) override;
        void removePipeline(Pipeline* pipeline) override;

        Image* createImage(TextureTarget target, renderer::Format format, const core::Dimension3D& dimension, u32 mipmapLevel, TextureUsageFlags flags) const override;
        //Image* createAttachmentImage(renderer::Format format, const core::Dimension3D& dimension, TextureSamples samples, TextureUsageFlags flags) const override;
        void removeImage(Image* image) override;

        Buffer* createBuffer(Buffer::BufferType type, u16 usageFlag, u64 size) override;
        void removeBuffer(Buffer* buffer) override;

        const DeviceCaps* getDeviceCaps() const override;

        VulkanStaginBufferManager* getStagingManager();

        static const std::vector<VkDynamicState>& getDynamicStates();
        static bool isDynamicState(VkDynamicState state);

        VulkanCommandBuffer* getOrCreateAndStartCommandBuffer(CommandTargetType type);

    private:

        VulkanDeviceCaps&   m_deviceCaps;
        DeviceInfo          m_deviceInfo;

        const std::string s_vulkanApplicationName = "VulkanGraphicContext";

        bool initialize() override;
        void destroy() override;

        void clearBackbuffer(const core::Vector4D & color) override;

        Framebuffer* createFramebuffer(const std::vector<Image*>& images, const core::Dimension2D& size) override;
        RenderPass* createRenderPass(const RenderPass::RenderPassInfo* renderpassInfo) override;
        Pipeline* createPipeline(Pipeline::PipelineType type) override;

        bool createInstance();
        bool createDevice();

        std::vector<VkQueue>        m_queueList;
        class VulkanSwapchain*      m_swapchain;

        VulkanCommandBufferManager* m_cmdBufferManager;
        VulkanDescriptorSetManager* m_descriptorSetManager;
        VulkanStaginBufferManager*  m_stagingBufferManager;
        VulkanUniformBufferManager* m_uniformBufferManager;

        VulkanMemory::VulkanMemoryAllocator* m_imageMemoryManager;
        VulkanMemory::VulkanMemoryAllocator* m_bufferMemoryManager;


        RenderPassManager*          m_renderpassManager;
        FramebufferManager*         m_framebuferManager;
        PipelineManager*            m_pipelineManager;

        //replace to another class
        struct CurrentContextState
        {
            CurrentContextState();

            void invalidateState();
            void invalidateCommandBuffer(CommandTargetType type);

            VulkanCommandBuffer* getAcitveBuffer(CommandTargetType type);
            bool isCurrentBufferAcitve(CommandTargetType type) const;

            VulkanCommandBuffer* _currentCmdBuffer[CommandTargetType::CommandTarget_Count];

        };
        //

        CurrentContextState         m_currentContextState;
        VulkanContextState*         m_currentContextStateNEW;

        static std::vector<VkDynamicState>  s_dynamicStates;

        bool prepareDraw(VulkanCommandBuffer* drawBuffer);

        const platform::Window* m_window;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
