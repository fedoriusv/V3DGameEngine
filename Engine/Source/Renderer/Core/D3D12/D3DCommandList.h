#pragma once

#include "Common.h"
#include "Renderer/Core/Pipeline.h"

#ifdef D3D_RENDER
#include "D3DConfiguration.h"
#include "D3DImage.h"
#include "D3DDeviceCaps.h"

namespace v3d
{
namespace renderer
{
    class Buffer;

namespace dx3d
{
    class D3DCommandListManager;
    class D3DImage;
    class D3DBuffer;
    class D3DFence;
    class D3DGraphicPipelineState;
    class D3DComputePipelineState;
    class D3DRenderTarget;
    class D3DResource;
    class D3DDescriptorHeap;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief D3DCommandList final class. DirectX Render side
    */
    class D3DCommandList
    {
    public:

        enum class Status
        {
            Initial,
            ReadyToRecord,
            Closed,
            Execute,
            Finish
        };

        enum Type
        {
            Direct = 0,
            Bundle,
            Compute,
            Copy,

            CountTypes
        };

        explicit D3DCommandList(ID3D12Device* device, Type type) noexcept;
        virtual ~D3DCommandList();

        ID3D12CommandList* getHandle() const;

        virtual void prepare() = 0;
        virtual void close() = 0;

        static D3D12_COMMAND_LIST_TYPE convertCommandListTypeToD3DType(D3DCommandList::Type type);

        void destroy();

        void setUsed(D3DResource* resource, u64 frame);

    protected:

        friend D3DCommandListManager;

        ID3D12Device* const m_device;

        ID3D12CommandAllocator* m_commandAllocator;
        bool m_ownAllocator;

        ID3D12CommandList* m_commandList;

        Status m_status;
        Type m_type;

    private:

        void init(ID3D12CommandList* cmdList, ID3D12CommandAllocator* allocator, bool own = true);
        bool checkOnComplete();

        D3DFence* m_fence;
        std::list<D3DResource*> m_resources;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * D3DGraphicsCommandList final class. DirectX Render side
    */
    class D3DGraphicsCommandList final : public D3DCommandList
    {
    public:

        explicit D3DGraphicsCommandList(ID3D12Device* device, Type type) noexcept;
        ~D3DGraphicsCommandList();

        void prepare() override;
        void close() override;

        void clearRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView, const f32 color[4], const std::vector<D3D12_RECT>& rect);
        void clearRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView, f32 depth, u32 stencil, D3D12_CLEAR_FLAGS flags, const std::vector<D3D12_RECT>& rect);

        void draw(u32 vertexCountPerInstance,u32 instanceCount, u32 startVertexLocation, u32 startInstanceLocation);
        void drawIndexed(u32 indexCountPerInstance, u32 instanceCount, u32 startIndexLocation, u32 baseVertexLocation, u32 startInstanceLocation);

        void dispatch(const core::Dimension3D& dimension);

        void setViewport(const std::vector<D3D12_VIEWPORT>& viewport);
        void setScissor(const std::vector<D3D12_RECT>& scissors);

        void resolve(D3DImage* image, const Image::Subresource& subresource, D3DImage* resolvedImage, const Image::Subresource& resolvedSubresource);

        void setRenderTarget(D3DRenderTarget* target);
        void setPipelineState(D3DGraphicPipelineState* pipeline);
        void setPipelineState(D3DComputePipelineState* pipeline);

        void setDescriptorTables(const std::vector<ID3D12DescriptorHeap*>& heaps, const std::map<u32, std::tuple<D3DDescriptorHeap*, u32>>& desc, Pipeline::PipelineType type);

        void setVertexState(u32 startSlot, const std::vector<u32>& strides, const std::vector<Buffer*>& buffers);
        void setIndexState(Buffer* buffer, DXGI_FORMAT format);

        void setViewInstanceMask(u32 mask);

        void transition(D3DImage* image, const Image::Subresource& subresource, D3D12_RESOURCE_STATES states, bool immediateTransition = false);

        template<class Resource>
        void transition(Resource* resource, D3D12_RESOURCE_STATES states, bool immediateTransition = false)
        {
            static_assert(std::is_same<Resource, D3DImage>() || std::is_same<Resource, D3DBuffer>(), "wrong type");
            ASSERT(m_commandList, "nullptr");
            ASSERT(m_status == Status::ReadyToRecord, "not record");

            if (resource->getState() != states)
            {
                this->setUsed(resource, 0);
                D3D12_RESOURCE_STATES oldState = resource->setState(states);

                D3D12_RESOURCE_BARRIER resourceBarrierDesc = {};
                resourceBarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                resourceBarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                resourceBarrierDesc.Transition.pResource = resource->getResource();
                resourceBarrierDesc.Transition.StateBefore = oldState;
                resourceBarrierDesc.Transition.StateAfter = states;
                resourceBarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

                m_transition.add(resourceBarrierDesc);
                if (D3DDeviceCaps::getInstance()->immediateTransition || immediateTransition)
                {
                    m_transition.execute(this);
                }
            }
        }

        ID3D12GraphicsCommandList1* getHandle() const;

    private:

        friend D3DCommandListManager;

        struct BarrierResources
        {
            void add(const D3D12_RESOURCE_BARRIER& resource);
            void execute(D3DGraphicsCommandList* commandList);

            std::vector<D3D12_RESOURCE_BARRIER> _barrierResources;
        };
        BarrierResources m_transition;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace dx3d
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER