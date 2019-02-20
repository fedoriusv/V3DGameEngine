#pragma once

#include "Common.h"

#include "DeviceCaps.h"
#include "TextureProperties.h"
#include "BufferProperties.h"
#include "ShaderProperties.h"

#include "RenderPass.h"
#include "Pipeline.h"
#include "Framebuffer.h"
#include "Buffer.h"
#include "Image.h"
#include "Sampler.h"
#include "ObjectTracker.h"

#include "Utils/NonCopyable.h"

namespace v3d
{
namespace platform
{
    class Window;
} //namespace platform

namespace renderer
{
    class Backbuffer;

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
        static void destroyContext(Context* context);

        RenderType getRenderType() const;
        
        //frame
        virtual void beginFrame() = 0;
        virtual void endFrame() = 0;
        virtual void presentFrame() = 0;
        virtual void submit(bool wait = false) = 0;

        virtual void clearBackbuffer(const core::Vector4D & color) = 0;

        //draw
        virtual void draw(StreamBufferDescription& desc, u32 firstVertex, u32 vertexCount, u32 firstInstance, u32 instanceCount) = 0;
        virtual void drawIndexed(StreamBufferDescription& desc, u32 firstIndex, u32 indexCount, u32 firstInstance, u32 instanceCount) = 0;

        //program bind
        virtual void bindImage(const Shader* shader, u32 bindIndex, const Image* image) = 0;
        virtual void bindSampler(const Shader* shader, u32 bindIndex, const SamplerDescription& desc) = 0;
        virtual void bindSampledImage(const Shader* shader, u32 bindIndex, const Image* image, const SamplerDescription& desc) = 0;
        virtual void bindUniformsBuffer(const Shader* shader, u32 bindIndex, u32 offset, u32 size, const void* data) = 0;

        //geometry bind
        //virtual void bindVertexBuffers(const std::vector<Buffer*>& buffer, const std::vector<u64>& offsets) = 0;

        //state
        virtual void setViewport(const core::Rect32& viewport, const core::Vector2D& depth = { 0.0f, 1.0f }) = 0;
        virtual void setScissor(const core::Rect32& scissor) = 0;

        virtual void setRenderTarget(const RenderPass::RenderPassInfo* renderpassInfo, const Framebuffer::FramebufferInfo* framebufferInfo) = 0;
        virtual void removeFramebuffer(Framebuffer* framebuffer) = 0;
        virtual void removeRenderPass(RenderPass* renderpass) = 0;
        virtual void invalidateRenderPass() = 0;

        virtual void setPipeline(const Pipeline::PipelineGraphicInfo* pipelineInfo) = 0;
        virtual void removePipeline(Pipeline* pipeline) = 0;

        //objects
        virtual Image* createImage(TextureTarget target, renderer::Format format, const core::Dimension3D& dimension, u32 mipmapLevel, TextureUsageFlags flags) const = 0;
        virtual void removeImage(Image* image) = 0;

        virtual Buffer* createBuffer(Buffer::BufferType type, u16 usageFlag, u64 size) = 0;
        virtual void removeBuffer(Buffer* buffer) = 0;

        u64 getCurrentFrameIndex() const;
        virtual const DeviceCaps* getDeviceCaps() const = 0;

    protected:

        friend RenderPassManager;
        friend FramebufferManager;
        friend PipelineManager;
        friend Backbuffer;
        friend SamplerManager;

        //management objects
        virtual Framebuffer* createFramebuffer(const std::vector<Image*>& attachments, const core::Dimension2D& size) = 0;
        virtual RenderPass* createRenderPass(const RenderPassDescription* renderpassDesc) = 0;
        virtual Pipeline* createPipeline(Pipeline::PipelineType type) = 0;
        virtual Sampler* createSampler() = 0;

        virtual bool initialize() = 0;
        virtual void destroy() = 0;


        struct BackbufferDesc
        {
            core::Dimension2D _size;
            Format            _format;
        };
        BackbufferDesc m_backufferDescription;

        RenderType  m_renderType;
        u64         m_frameCounter;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
