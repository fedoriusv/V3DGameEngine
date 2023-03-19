#pragma once

#include "Common.h"

#include "Utils/Profiler.h"

#include "Renderer/DeviceCaps.h"
#include "Renderer/TextureProperties.h"
#include "Renderer/BufferProperties.h"
#include "Renderer/ShaderProperties.h"
#include "Renderer/PipelineStateProperties.h"
#include "Renderer/ObjectTracker.h"
#include "Renderer/Core/RenderFrameProfiler.h"

#include "Image.h"
#include "Sampler.h"
#include "Buffer.h"
#include "Query.h"
#include "RenderPass.h"
#include "Framebuffer.h"
#include "Pipeline.h"

namespace v3d
{
namespace platform
{
    class Window;
} //namespace platform

namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Backbuffer;
    class PipelineState;
    class SamplerState;
    class RenderTargetState;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Context interface class
    */
    class Context
    {

    public:

        /**
        * @brief RenderType enum
        */
        enum class RenderType
        {
            EmptyRender,
            VulkanRender,
            DirectXRender
        };

        /**
        * @brief DeviceMask enum
        */
        enum DeviceMask
        {
            GraphicMask = 0x1,
            ComputeMask = 0x2,
            TransferMask = 0x4
        };

        /**
        * @brief createContext static function. Used for create context.
        * Must be called in game thread
        * @param const platform::Window* window [required]
        * @param RenderType type [required]
        * @param DeviceMask mask [optional]
        * @return Context pointer
        */
        [[nodiscard]] static Context* createContext(platform::Window* window, RenderType type, DeviceMask mask = DeviceMask::GraphicMask);

        /**
        * @brief destroyContext static function. Used for destroy context.
        * Must be called in game thread
        * @param Context* context [required]
        */
        static void destroyContext(Context* context);

        /**
        * @brief Gets render type.
        * Suppots next types:
        *  EmptyRender,
        *  VulkanRender,
        *  DirectXRender
        * @return RenderType
        */
        RenderType getRenderType() const;

        /**
        * @brief getDeviceCaps
        * @return Graphic capabilities for current device
        */
        virtual const DeviceCaps* getDeviceCaps() const = 0;
        
        /**
        * @brief frame beginFrame/endFrame
        */
        virtual void beginFrame() = 0;
        virtual void endFrame() = 0;

        /**
        * @brief frame presentFrame
        */
        virtual void presentFrame() = 0;

        /**
        * @brief frame submit
        * @param bool wait [optional]
        */
        virtual void submit(bool wait = false) = 0;

        /**
        * @brief query begin/end commands. Only for occlusions queries
        * Can be called inside/outside renderpass
        * @param const Query* query [required]
        * @param const u32 id [required]
        * @param const std::string& tag [optional]
        */
        virtual void beginQuery(const Query* query, u32 id, const std::string& tag = "") = 0;
        virtual void endQuery(const Query* query, u32 id) = 0;

        /**
        * @brief query timestamp command. Only for timestamp queries
        * Can be called inside/outside renderpass
        * @param const Query* query [required]
        * @param const u32 id [required]
        * @param const std::string& tag [optional]
        */
        virtual void timestampQuery(const Query* query, u32 id, const std::string& tag = "") = 0;

        /**
        * @brief draw command by vertex
        * Must be called inside frame & renderpass
        * @param const StreamBufferDescription& desc [required]
        * @param u32 firstVertex [required]
        * @param u32 vertexCount [required]
        * @param u32 firstInstance [required]
        * @param u32 instanceCount [required]
        */
        virtual void draw(const StreamBufferDescription& desc, u32 firstVertex, u32 vertexCount, u32 firstInstance, u32 instanceCount) = 0;

        /**
        * @brief draw command by index
        * Must be called inside frame & renderpass
        * @param const StreamBufferDescription& desc [required]
        * @param u32 firstIndex [required]
        * @param u32 indexCount [required]
        * @param u32 firstInstance [required]
        * @param u32 instanceCount [required]
        */
        virtual void drawIndexed(const StreamBufferDescription& desc, u32 firstIndex, u32 indexCount, u32 firstInstance, u32 instanceCount) = 0;

        /**
        * @brief compute command
        * @param const core::Dimension3D& groups [required]
        */
        virtual void dispatchCompute(const core::Dimension3D& groups) = 0;

        //program bind
        virtual void bindImage(const Shader* shader, u32 bindIndex, const Image* image, s32 layer = k_generalLayer, s32 mip = k_allMipmapsLevels) = 0;
        virtual void bindSampler(const Shader* shader, u32 bindIndex, const Sampler::SamplerInfo* samplerInfo) = 0;
        virtual void bindSampledImage(const Shader* shader, u32 bindIndex, const Image* image, const Sampler::SamplerInfo* samplerInfo, s32 layer = k_generalLayer, s32 mip = k_allMipmapsLevels) = 0; //TODO remove
        virtual void bindUniformsBuffer(const Shader* shader, u32 bindIndex, u32 offset, u32 size, const void* data) = 0;
        virtual void bindStorageImage(const Shader* shader, u32 bindIndex, const Image* image, s32 layer = k_generalLayer, s32 mip = k_allMipmapsLevels) = 0;

        //transfer
        virtual void transitionImages(std::vector<std::tuple<const Image*, Image::Subresource>>& images, TransitionOp transition) = 0;

        /**
        * @brief setViewport command
        * @param const core::Rect32& viewport [required]
        * @param const core::Vector2D& depth [required]
        */
        virtual void setViewport(const core::Rect32& viewport, const core::Vector2D& depth = { 0.0f, 1.0f }) = 0;

        /**
        * @brief setScissor command
        * @param const core::Rect32& scissor [required]
        */
        virtual void setScissor(const core::Rect32& scissor) = 0;

        virtual void setRenderTarget(const RenderPass::RenderPassInfo* renderpassInfo, const Framebuffer::FramebufferInfo* framebufferInfo) = 0;
        virtual void invalidateRenderTarget() = 0;

        /**
        * @brief setPipeline command
        * Set Current pipeline state for graphic pipeline
        * @param const Pipeline::PipelineGraphicInfo* pipelineInfo [required]
        */
        virtual void setPipeline(const Pipeline::PipelineGraphicInfo* pipelineInfo) = 0;

        /**
        * @brief setPipeline command
        * Set Current pipeline state for compute pipeline
        * @param const Pipeline::PipelineComputeInfo* pipelineInfo [required]
        */
        virtual void setPipeline(const Pipeline::PipelineComputeInfo* pipelineInfo) = 0;

        virtual void clearRenderTarget(const std::vector<const Image*>& images, Framebuffer::ClearValueInfo& clearValues) = 0;

        //objects
        [[nodiscard]] virtual Image* createImage(TextureTarget target, Format format, const core::Dimension3D& dimension, u32 layers, u32 mipmapLevel, TextureUsageFlags flags, [[maybe_unused]] const std::string& name = "") = 0;
        [[nodiscard]] virtual Image* createImage(TextureTarget target, Format format, const core::Dimension3D& dimension, u32 layers, TextureSamples samples, TextureUsageFlags flags, [[maybe_unused]] const std::string& name = "") = 0;
        virtual void removeImage(Image* image) = 0;

        [[nodiscard]] virtual Buffer* createBuffer(Buffer::BufferType type, u16 usageFlag, u64 size, [[maybe_unused]] const std::string& name = "") = 0;
        virtual void removeBuffer(Buffer* buffer) = 0;

        [[nodiscard]] virtual Sampler* createSampler(const SamplerDescription& desc) = 0;
        virtual void removeSampler(Sampler* sampler) = 0;

        [[nodiscard]] virtual Framebuffer* createFramebuffer(const std::vector<Image*>& attachments, const core::Dimension2D& size) = 0;
        virtual void removeFramebuffer(Framebuffer* framebuffer) = 0;

        [[nodiscard]] virtual RenderPass* createRenderPass(const RenderPassDescription* renderpassDesc) = 0;
        virtual void removeRenderPass(RenderPass* renderpass) = 0;

        [[nodiscard]] virtual Pipeline* createPipeline(Pipeline::PipelineType type, [[maybe_unused]] const std::string& name = "") = 0;
        virtual void removePipeline(Pipeline* pipeline) = 0;

        [[nodiscard]] virtual Query* createQuery(QueryType type, u32 count, const Query::QueryRespose& callback, [[maybe_unused]] const std::string& name = "") = 0;
        virtual void removeQuery(Query* query) = 0;

        virtual void clearBackbuffer(const core::Vector4D& color) = 0;
        virtual void generateMipmaps(Image* image, u32 layer, TransitionOp state) = 0;

        /**
        * @brief getCurrentFrameIndex
        * @return Current frame index
        */
        u64 getCurrentFrameIndex() const;

        /**
        * @brief getBackbufferSize
        * @return Backbuffer size
        */
        const core::Dimension2D& getBackbufferSize() const;

        /**
        * @brief getBackbufferFormat
        * @return Backbuffer image format
        */
        Format getBackbufferFormat() const;

    protected:

        Context() noexcept;
        virtual ~Context();

        Context(const Context&) = delete;
        Context& operator=(const Context&) = delete;

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

#if FRAME_PROFILER_ENABLE
        utils::ProfileManager m_frameProfiler;
        RenderFrameProfiler* m_CPUProfiler;
#endif //FRAME_PROFILER_ENABLE

        friend Backbuffer;
        friend PipelineState;
        friend SamplerState;
        friend RenderTargetState;
    };

    inline Context::RenderType Context::getRenderType() const
    {
        return m_renderType;
    }

    inline u64 Context::getCurrentFrameIndex() const
    {
        return m_frameCounter;
    }

    inline const core::Dimension2D& Context::getBackbufferSize() const
    {
        return m_backufferDescription._size;
    }

    inline Format Context::getBackbufferFormat() const
    {
        return m_backufferDescription._format;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
