#pragma once

#include "Renderer/Context.h"
#include "VulkanDeviceCaps.h"
#include "VulkanCommandBufferManager.h"

#ifdef VULKAN_RENDER
#include "VulkanWrapper.h"

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

        //commands
        void beginFrame() override;
        void endFrame() override;
        void presentFrame() override;

        void clearBackbuffer(const core::Vector4D & color) override;

        //states
        void setViewport(const core::Rect32& viewport) override;

        void setRenderTarget(const RenderPass::RenderPassInfo* renderpassInfo, const std::vector<Image*>& attachments, const RenderPass::ClearValueInfo* clearInfo) override;
        void removeRenderTarget(const RenderPass::RenderPassInfo * renderpassInfo, const std::vector<Image*>& attachments, const RenderPass::ClearValueInfo * clearInfo) override;

        void setPipeline(const GraphicsPipelineState::GraphicsPipelineStateDesc* pipelineInfo,
            const ShaderProgram::ShaderProgramInfo* programInfo, const RenderPass::RenderPassInfo* renderpassInfo) override;

        Image* createImage(TextureTarget target, renderer::Format format, const core::Dimension3D& dimension, u32 mipmapLevel, s16 filter, TextureAnisotropic anisotropicLevel, TextureWrap wrap) const override;
        Image* createAttachmentImage(renderer::Format format, const core::Dimension3D& dimension, TextureSamples samples, s16 filter, TextureAnisotropic anisotropicLevel, TextureWrap wrap) const override;

        VulkanCommandBuffer* getCurrentBuffer(VulkanCommandBufferManager::CommandTargetType type) const;

        void transferImageLayout(VulkanImage* image, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImageLayout layout) const;

        const DeviceCaps* getDeviceCaps() const override;

    private:

        VulkanDeviceCaps&   m_deviceCaps;
        DeviceInfo          m_deviceInfo;

        const std::string s_vulkanApplicationName = "VulkanGraphicContext";

        bool initialize() override;
        void destroy() override;

        Framebuffer* createFramebuffer(const std::vector<Image*>& images, const core::Dimension2D& size) override;
        RenderPass* createRenderPass(const RenderPass::RenderPassInfo* renderpassInfo) override;
        Pipeline* createPipeline(const GraphicsPipelineState::GraphicsPipelineStateDesc* pipelineInfo,
            const ShaderProgram::ShaderProgramInfo* programInfo, const RenderPass::RenderPassInfo* renderpassInfo) override;

        bool createInstance();
        bool createDevice();

        std::vector<VkQueue>    m_queueList;
        class VulkanSwapchain*  m_swapchain;

        VulkanCommandBufferManager* m_drawCmdBufferManager;
        class VulkanMemory*         m_memoryManager;
        RenderPassManager*          m_renderpassManager;
        FramebufferManager*         m_framebuferManager;
        PipelineManager*            m_pipelineManager;

        struct CurrentContextState
        {
            VulkanCommandBuffer* _currentDrawBuffer;

            VulkanRenderPass*    _currentRenderpass;
            VulkanFramebuffer*   _currentFramebuffer;

            VulkanGraphicPipeline* _currentPipeline;
        };

        CurrentContextState     m_currentContextState;

        const platform::Window* m_window;

        u64 m_frameCounter;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
