#pragma once

#include "Render.h"
#include "DeviceCaps.h"

#include "Texture.h"
#include "Buffer.h"
#include "Swapchain.h"
#include "SamplerState.h"
#include "RenderTargetState.h"
#include "PipelineState.h"

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

    protected:

        CmdList() noexcept = default;
        virtual ~CmdList() = default;

        Type m_type;
    };


    class CmdListCompute : public CmdList
    {
    protected:

        CmdListCompute() noexcept
        {
            m_type = Type::Compute;
        }
        
        ~CmdListCompute() = default;
    };


    class CmdListRender : public CmdListCompute
    {
    public:

        CmdListRender() noexcept
        {
            m_type = Type::Render;
        }
        
        ~CmdListRender() = default;

        /**
        * @brief setViewport command
        * @param const math::Rect32& viewport [required]
        * @param const math::Vector2D& depth [required]
        */
        virtual void setViewport(const math::Rect32& viewport, const math::Vector2D& depth = { 0.0f, 1.0f }) = 0;

        /**
        * @brief setScissor command
        * @param const math::Rect32& scissor [required]
        */
        virtual void setScissor(const math::Rect32& scissor) = 0;

        /**
        */
        virtual void setStencilRef(u32 mask) = 0;

        /**
        */
        virtual void beginRenderTarget(const RenderTargetState& rendertarget) = 0;

        /**
        */
        virtual void endRenderTarget() = 0;

        /**
        */
        virtual void setPipeline(GraphicsPipelineState& pipeline) = 0;

        /**
        */
        virtual void transition(const TextureView& texture, TransitionOp state) = 0;

        /**
        */
        virtual void bindTexture(u32 binding, Texture* texture) = 0;
        virtual void bindBuffer(u32 binding, Buffer* buffer) = 0;
        virtual void bindSampler(u32 binding, SamplerState* sampler) = 0;
        virtual void bindConstantBuffer(u32 binding, u32 size, void* data) = 0;

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
        virtual void drawIndexed(const GeometryBufferDesc& desc, u32 firstIndex, u32 indexCount, u32 firstInstance, u32 instanceCount) = 0;

        /**
        */
        virtual void clear(Texture* texture, const render::Color& color) = 0;

        /**
        */
        virtual void clear(Texture* texture, f32 depth, u32 stencil) = 0;


        //virtual bool uploadTexture(CmdList* cmd, Texture* texture) = 0;
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
            EmptyRender,
            VulkanRender,
            DirectXRender
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
        */
        template<class TCmdList>
        TCmdList* createCommandList(DeviceMask queueType);

        /**
        */
        virtual void destroyCommandList(CmdList* cmdList) = 0;

        /**
        */
        virtual Swapchain* createSwapchain(platform::Window* window, const Swapchain::SwapchainParams& params) = 0;

        /**
        */
        virtual void destroySwapchain(Swapchain* swapchain) = 0;

        //virtual Texture2D* createTexture2D() = 0;
        //virtual Texture3D* createTexture3D() = 0;
        //virtual Texture2DArray* createTexture2DArray() = 0;
        //virtual bool destroyTexture(Texture* texture) = 0;

    protected:

        Device() noexcept;
        virtual ~Device() = default;

        Device(const Device&) = delete;
        Device& operator=(const Device&) = delete;

        virtual CmdList* createCommandList_Impl(DeviceMask queueType) = 0;

        virtual bool initialize() = 0;
        virtual void destroy() = 0;

        RenderType  m_renderType;
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
