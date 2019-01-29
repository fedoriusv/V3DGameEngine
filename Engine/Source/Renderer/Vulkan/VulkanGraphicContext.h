#pragma once

#include "Renderer/Context.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"
#include "VulkanDeviceCaps.h"
#include "VulkanDescriptorSet.h"
#include "VulkanCommandBufferManager.h"
#include "VulkanContextState.h"

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

    class VulkanStaginBufferManager;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    struct DeviceInfo
    {
        VkInstance          _instance;
        VkPhysicalDevice    _physicalDevice;
        u32                 _queueFamilyIndex;
        VkDevice            _device;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

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

        void clearBackbuffer(const core::Vector4D & color) override;


        void bindTexture(const resource::Shader* shader, const std::string& name, const Image* image) override;
        void bindUniformBuffers(const resource::Shader* shader, const std::string& name, const void* data, u32 offset, u32 size) override;

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

        Image* createImage(TextureTarget target, renderer::Format format, const core::Dimension3D& dimension, u32 mipmapLevel, s16 filter, TextureAnisotropic anisotropicLevel, TextureWrap wrap) const override;
        Image* createAttachmentImage(renderer::Format format, const core::Dimension3D& dimension, TextureSamples samples, s16 filter, TextureAnisotropic anisotropicLevel, TextureWrap wrap) const override;
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

        Framebuffer* createFramebuffer(const std::vector<Image*>& images, const core::Dimension2D& size) override;
        RenderPass* createRenderPass(const RenderPass::RenderPassInfo* renderpassInfo) override;
        Pipeline* createPipeline(Pipeline::PipelineType type) override;

        bool createInstance();
        bool createDevice();

        std::vector<VkQueue>        m_queueList;
        class VulkanSwapchain*      m_swapchain;

        VulkanCommandBufferManager* m_cmdBufferManager;
        VulkanStaginBufferManager*  m_stagingBufferManager;
        VulkanDescriptorSetManager* m_descriptorSetManager;

        class VulkanMemory*         m_imageMemoryManager;
        class VulkanMemory*         m_bufferMemoryManager;


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

            bool isCurrentRenderPass(const RenderPass* pass) const;
            bool isCurrentFramebuffer(const Framebuffer* framebuffer) const;
            bool isCurrentPipeline(const Pipeline* pipeline) const;

            VulkanRenderPass*    _currentRenderpass;
            VulkanFramebuffer*   _currentFramebuffer;

            VulkanGraphicPipeline* _currentPipeline;
            bool _updatePipeline;

            std::vector<DescriptorBinding> _descriptorsStates;
            std::pair<StreamBufferDescription, bool> _currentStreamBufferDescription;
            std::tuple<std::vector<Buffer*>, std::vector<u64>, bool> _boundUniformBuffers;

            void* _boundShaderStage[ShaderType::ShaderType_Count];
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
