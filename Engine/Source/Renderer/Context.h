#pragma once

#include "Common.h"
#include "TextureProperties.h"
#include "DeviceCaps.h"
#include "RenderPass.h"
#include "Pipeline.h"
#include "Framebuffer.h"
#include "ObjectTracker.h"
#include "Object/ShaderProgram.h"

#include "Utils/NonCopyable.h"

namespace v3d
{
namespace platform
{
    class Window;
} //namespace platform

namespace renderer
{
    class Image;
    class Framebuffer;
    class Pipeline;

    struct PipelineGraphicInfo;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Context interface class
    */
    class Context : public utils::NonCopyable
    {

    public:

        enum DeviceMask
        {
            GraphicMask = 0x1,
            ComputeMask = 0x2,
            TransferMask = 0x4
        };

        enum class RenderType
        {
            EmptyRender,
            VulkanRender,
        };

        Context() noexcept;
        virtual ~Context();

        static Context* createContext(const platform::Window* window, RenderType type, DeviceMask mask = DeviceMask::GraphicMask);

        RenderType getRenderType() const;
        
        //frame
        virtual void beginFrame() = 0;
        virtual void endFrame() = 0;
        virtual void presentFrame() = 0;


        virtual void clearBackbuffer(const core::Vector4D & color) = 0;

        virtual void setViewport(const core::Rect32& viewport) = 0;

        virtual void setRenderTarget(const RenderPass::RenderPassInfo* renderpassInfo, const std::vector<Image*>& attachments, const RenderPass::ClearValueInfo* clearInfo, 
            const std::tuple<ObjectTracker<RenderPass>*, ObjectTracker<Framebuffer>*>& trackers) = 0;
        virtual void removeRenderTarget(const RenderPass::RenderPassInfo * renderpassInfo, const std::vector<Image*>& attachments, const RenderPass::ClearValueInfo * clearInfo) = 0;
        virtual void removeFramebuffer(Framebuffer* framebuffer) = 0;
        virtual void removeRenderPass(RenderPass* renderpass) = 0;

        virtual void setPipeline(const Pipeline::PipelineGraphicInfo* pipelineInfo, ObjectTracker<Pipeline>* tracker) = 0;
        virtual void removePipeline(Pipeline* pipeline) = 0;

        //create
        virtual Image* createImage(TextureTarget target, renderer::Format format, const core::Dimension3D& dimension, u32 mipmapLevel,
            s16 filter, TextureAnisotropic anisotropicLevel, TextureWrap wrap) const = 0;

        virtual Image* createAttachmentImage(renderer::Format format, const core::Dimension3D& dimension, TextureSamples samples,
            s16 filter, TextureAnisotropic anisotropicLevel, TextureWrap wrap) const = 0;

        virtual const DeviceCaps* getDeviceCaps() const = 0;

    protected:

        friend RenderPassManager;
        friend FramebufferManager;
        friend PipelineManager;

        virtual Framebuffer* createFramebuffer(const std::vector<Image*>& attachments, const core::Dimension2D& size) = 0;
        virtual RenderPass* createRenderPass(const RenderPass::RenderPassInfo* renderpassInfo) = 0;
        virtual Pipeline* createPipeline(Pipeline::PipelineType type) = 0;

        virtual bool initialize() = 0;
        virtual void destroy() = 0;

        RenderType  m_renderType;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
