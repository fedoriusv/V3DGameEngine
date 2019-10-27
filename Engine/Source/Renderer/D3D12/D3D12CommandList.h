#pragma once

#ifdef D3D_RENDER
#   include "D3D12Configuration.h"

namespace v3d
{
namespace renderer
{
namespace d3d12
{
    class D3DCommandListManager;
    class D3DImage;
    class D3DFence;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * D3DCommandList final class. DirectX Render side
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

        static D3D12_COMMAND_LIST_TYPE convertCommandListTypeToD3DType(Type type);

        void destroy();

    protected:

        friend D3DCommandListManager;

        ComPtr<ID3D12Device> m_device;

        ComPtr<ID3D12CommandAllocator> m_commandAllocator;
        bool m_ownAllocator;

        ComPtr<ID3D12CommandList> m_commandList;

        Status m_status;
        Type m_type;

        void resetFence();

    private:

        void init(ID3D12CommandList* cmdList, ID3D12CommandAllocator* allocator, bool own = true);
        mutable std::atomic<u64> m_fenceValue;
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

        void clearRenderTarget(D3DImage* image, const f32 color[4], const std::vector<D3D12_RECT>& rect);
        void clearRenderTarget(D3DImage* image, f32 depth, u32 stencil, D3D12_CLEAR_FLAGS flags, const std::vector<D3D12_RECT>& rect);

        void setViewport(const std::vector<D3D12_VIEWPORT>& viewport);
        void setScissor(const std::vector<D3D12_RECT>& scissors);

        void setRenderTarget();

        void transition(D3DImage* image, D3D12_RESOURCE_STATES state);

        ID3D12GraphicsCommandList* getHandle() const;

    private:

        friend D3DCommandListManager;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace d3d12
} //namespace renderer
} //namespace v3d


#endif //D3D_RENDER
