#pragma once

#include "Render.h"
#include "DeviceCaps.h"
#include "RenderFrameProfiler.h"
#include "Utils/Profiler.h"

#include "Texture.h"
#include "Buffer.h"
#include "Swapchain.h"
#include "SamplerState.h"
#include "RenderTargetState.h"
#include "PipelineState.h"
#include "ObjectTracker.h"

namespace v3d
{
namespace platform
{
    class Window;
} //namespace platform
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief CmdList interface class
    */
    class CmdList : public utils::ResourceID<CmdList, u32>
    {
    public:

        enum class Type
        {
            Render,
            Compute,
            Transfer
        };

        /**
         * @brief transition command
        */
        virtual void transition(const TextureView& textureView, TransitionOp state) = 0;

        /**
         * @brief bindDescriptorSet command
        */
        virtual void bindDescriptorSet(u32 set, const std::vector<Descriptor>& descriptors) = 0;

        virtual void bindTexture(u32 set, u32 slot, const TextureView& textureView) = 0;
        virtual void bindBuffer(u32 set, u32 slot, Buffer* buffer) = 0;
        virtual void bindSampler(u32 set, u32 slot, SamplerState* sampler) = 0;
        virtual void bindConstantBuffer(u32 set, u32 slot, u32 size, void* data) = 0;

    protected:

        CmdList() noexcept
            : m_type(Type::Render)
        {
        }

        virtual ~CmdList() = default;

        Type m_type;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief CmdListCompute interface class
    */
    class CmdListCompute : public CmdList
    {
    public:

        virtual void setPipelineState(ComputePipelineState& pipeline) = 0;

    protected:

        CmdListCompute() noexcept
        {
            m_type = Type::Compute;
        }
        
        ~CmdListCompute() = default;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief CmdListRender interface class
    */
    class CmdListRender : public CmdListCompute
    {
    public:

        /**
        * @brief setViewport command
        * @param const math::Rect32& viewport [required]
        * @param const math::Vector2D& depth [optional]
        */
        virtual void setViewport(const math::Rect32& viewport, const math::Vector2D& depth = { 0.0f, 1.0f }) = 0;

        /**
        * @brief setScissor command
        * @param const math::Rect32& scissor [required]
        */
        virtual void setScissor(const math::Rect32& scissor) = 0;

        /**
        * @brief setStencilRef command
        * @param const u32 mask [required]
        */
        virtual void setStencilRef(u32 mask) = 0;

        /**
        */
        virtual void beginRenderTarget(RenderTargetState& rendertarget) = 0;

        /**
        */
        virtual void endRenderTarget() = 0;

        /**
        */
        virtual void setPipelineState(GraphicsPipelineState& pipeline) = 0;

        /**
        * @brief draw command by vertex
        * Must be called inside frame & renderpass
        * @param const VertexBufferDesc& desc [required]
        * @param u32 firstVertex [required]
        * @param u32 vertexCount [required]
        * @param u32 firstInstance [required]
        * @param u32 instanceCount [required]
        */
        virtual void draw(const GeometryBufferDesc& desc, u32 firstVertex, u32 vertexCount, u32 firstInstance, u32 instanceCount) = 0;

        /**
        * @brief draw command by index
        * Must be called inside frame & renderpass
        * @param const VertexBufferDesc& desc [required]
        * @param u32 firstIndex [required]
        * @param u32 indexCount [required]
        * @param u32 firstInstance [required]
        * @param u32 instanceCount [required]
        */
        virtual void drawIndexed(const GeometryBufferDesc& desc, u32 firstIndex, u32 indexCount, u32 vertexOffest, u32 firstInstance, u32 instanceCount) = 0;

        /**
         * @brief clear command
        */
        virtual void clear(Texture* texture, const color::Color& color) = 0;

        /**
         * @brief clear command
        */
        virtual void clear(Texture* texture, f32 depth, u32 stencil) = 0;

        /**
         * @brief upload command
        */
        virtual bool uploadData(Texture2D* texture, u32 size, const void* data) = 0;
        virtual bool uploadData(Texture3D* texture, u32 size, const void* data) = 0;

        /**
         * @brief upload command
        */
        virtual bool uploadData(Buffer* buffer, u32 offset, u32 size, const void* data) = 0;

    protected:

        CmdListRender() noexcept
        {
            m_type = Type::Render;
        }

        ~CmdListRender() = default;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief SyncPoint interface class
    */
    class SyncPoint
    {
    protected:

        SyncPoint() = default;
        virtual ~SyncPoint() = default;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Device interface class
    */
    class Device
    {
    public:

        /**
        * @brief RenderType enum
        */
        enum class RenderType
        {
            Empty,
            Vulkan,
            DirectX
        };

        /**
        * @brief DeviceMask enum.
        * If device is supported physical queue will be created for every mask
        */
        enum DeviceMask
        {
            GraphicMask = 0x1,
            ComputeMask = 0x2,
            TransferMask = 0x4
        };
        using DeviceMaskFlags = u32;

        /**
        * @brief createDevice static function. Used for creating device context.
        * Must be called in game thread
        * @param const platform::Window* window [required]
        * @param RenderType type [required]
        * @param DeviceMask mask [optional]
        * @return Context pointer
        */
        [[nodiscard]] static Device* createDevice(RenderType type, DeviceMaskFlags mask = DeviceMask::GraphicMask);

        /**
        * @brief destroyDevice static function. Used for destroying device context.
        * Must be called in game thread
        * @param DeviceContext* device [required]
        */
        static void destroyDevice(Device* device);

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
        * @brief get device capabilities
        * @return DeviceCaps
        */
        virtual const DeviceCaps& getDeviceCaps() const = 0;

        /**
        * @brief frame submit
        * @param bool wait [optional]
        */
        virtual void submit(CmdList* cmd, bool wait = false) = 0;

        /**
        * @brief frame submit
        * @param bool wait [optional]
        */
        virtual void submit(CmdList* cmd, SyncPoint* sync, bool wait = false) = 0;

        /**
        * @brief waitGPUCompletion
        * @param CmdList* cmd
        */
        virtual void waitGPUCompletion(CmdList* cmd) = 0;

        /**
        * @brief createCommandList
        */
        template<class TCmdList>
        [[nodiscard]] TCmdList* createCommandList(DeviceMask queueType);

        /**
        * @brief destroyCommandList
        */
        virtual void destroyCommandList(CmdList* cmd) = 0;

        /**
        * @brief createSwapchain
        */
        [[nodiscard]] virtual Swapchain* createSwapchain(platform::Window* window, const Swapchain::SwapchainParams& params) = 0;

        /**
        * @brief destroySwapchain
        */
        virtual void destroySwapchain(Swapchain* swapchain) = 0;

        /**
        * @brief createSyncPoint
        */
        [[nodiscard]] virtual SyncPoint* createSyncPoint(CmdList* cmd) = 0;

        /**
        * @brief destroySyncPoint
        */
        virtual void destroySyncPoint(CmdList* cmd, SyncPoint* sync) = 0;

    public:

        [[nodiscard]] virtual TextureHandle createTexture(TextureTarget target, Format format, const math::Dimension3D& dimension, u32 layers, u32 mipmapLevel, TextureUsageFlags flags, const std::string& name = "") = 0;
        [[nodiscard]] virtual TextureHandle createTexture(TextureTarget target, Format format, const math::Dimension3D& dimension, u32 layers, TextureSamples samples, TextureUsageFlags flags, const std::string& name = "") = 0;
        virtual void destroyTexture(TextureHandle texture) = 0;

        [[nodiscard]] virtual BufferHandle createBuffer(RenderBuffer::Type type, u16 usageFlag, u64 size, const std::string& name = "") = 0;
        virtual void destroyBuffer(BufferHandle buffer) = 0;

    public:

        virtual void destroyFramebuffer(Framebuffer* framebuffer) = 0;
        virtual void destroyRenderpass(RenderPass* renderpass) = 0;
        virtual void destroyPipeline(RenderPipeline* pipeline) = 0;
        virtual void destroySampler(Sampler* sampler) = 0;

    protected:

        Device() noexcept;
        virtual ~Device() = default;

        Device(const Device&) = delete;
        Device& operator=(const Device&) = delete;

        virtual CmdList* createCommandList_Impl(DeviceMask queueType) = 0;

        virtual bool initialize() = 0;
        virtual void destroy() = 0;

        RenderType              m_renderType;
#if FRAME_PROFILER_ENABLE
        utils::ProfileManager   m_frameProfiler;
#endif //FRAME_PROFILER_ENABLE
    };

    inline Device::RenderType Device::getRenderType() const
    {
        return m_renderType;
    }

    template<class TCmdList>
    inline TCmdList* Device::createCommandList(DeviceMask queueType)
    {
        return static_cast<TCmdList*>(createCommandList_Impl(queueType));
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
