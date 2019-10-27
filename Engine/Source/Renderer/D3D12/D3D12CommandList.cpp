#include "D3D12CommandList.h"
#include "Utils/Logger.h"

#ifdef D3D_RENDER
#   include "d3dx12.h"
#   include "D3D12Debug.h"
#   include "D3D12Image.h"

namespace v3d
{
namespace renderer
{
namespace d3d12
{

D3D12_COMMAND_LIST_TYPE D3DCommandList::convertCommandListTypeToD3DType(Type type)
{
    switch (type)
    {
    case Type::Direct:
        return D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT;

    case Type::Bundle:
        return D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_BUNDLE;

    case Type::Compute:
        return D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_COMPUTE;

    case Type::Copy:
        return D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_COPY;

    default:
        ASSERT(false, "unsupported type");
    }

    return D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT;
}

void D3DCommandList::destroy()
{
    ASSERT(m_status == Status::Finish, "not finished");
    D3DCommandList::resetFence();
    if (m_commandList)
    {
        m_commandList = nullptr;
    }

    if (m_commandAllocator)
    {
        m_commandAllocator = nullptr;
    }
}

D3DCommandList::D3DCommandList(ID3D12Device* device, Type type) noexcept
    : m_device(device)
    , m_commandList(nullptr)

    , m_commandAllocator(nullptr)
    , m_ownAllocator(true)

    , m_status(Status::Initial)
    , m_type(type)

    , m_fenceValue(0U)
{
    LOG_DEBUG("D3DCommandList::D3DCommandList constructor %llx", this);
}

D3DCommandList::~D3DCommandList()
{
    LOG_DEBUG("D3DCommandList::~D3DCommandList destructor %llx", this);
    ASSERT(m_status == Status::Finish, "not finished");

    ASSERT(!m_commandList, "not nullptr");
    ASSERT(!m_commandAllocator, "not nullptr");
}

ID3D12CommandList* D3DCommandList::getHandle() const
{
    ASSERT(m_commandList, "nullptr");
    return m_commandList.Get();
}

void D3DCommandList::init(ID3D12CommandList* cmdList, ID3D12CommandAllocator* allocator, bool own)
{
    m_commandAllocator = allocator;
    m_ownAllocator = own;

    m_commandList = cmdList;
}

void D3DCommandList::resetFence()
{
    m_fenceValue.store(0U);
}


D3DGraphicsCommandList::D3DGraphicsCommandList(ID3D12Device* device, Type type) noexcept
    : D3DCommandList(device, type)
{
    LOG_DEBUG("D3DGraphicsCommandList constructor %llx", this);
}

D3DGraphicsCommandList::~D3DGraphicsCommandList()
{
    LOG_DEBUG("D3DGraphicsCommandList destructor %llx", this);
}

void D3DGraphicsCommandList::prepare()
{
    ASSERT(m_commandList, "nullptr");
    if (m_ownAllocator)
    {
        HRESULT result = m_commandAllocator->Reset();
        ASSERT(SUCCEEDED(result), "error");
    }

    {
        HRESULT result = D3DGraphicsCommandList::getHandle()->Reset(m_commandAllocator.Get(), nullptr);
        ASSERT(SUCCEEDED(result), "error");
    }

    m_status = Status::ReadyToRecord;
}

void D3DGraphicsCommandList::close()
{
    ASSERT(m_commandList, "nullptr");

    HRESULT result = D3DGraphicsCommandList::getHandle()->Close();
    ASSERT(SUCCEEDED(result), "error");

    m_status = Status::Closed;
}

void D3DGraphicsCommandList::transition(D3DImage* image, D3D12_RESOURCE_STATES state)
{
    ASSERT(m_commandList, "nullptr");
    ASSERT(m_status == Status::ReadyToRecord, "not record");

    if (image->getState() != state)
    {
        ID3D12GraphicsCommandList* cmdList = D3DGraphicsCommandList::getHandle();

        D3D12_RESOURCE_STATES oldState = image->setState(state);
        cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(image->getResource(), oldState, state));
    }
}

ID3D12GraphicsCommandList* D3DGraphicsCommandList::getHandle() const
{
    ASSERT(m_commandList, "nullptr");
    return static_cast<ID3D12GraphicsCommandList*>(m_commandList.Get());
}

void D3DGraphicsCommandList::clearRenderTarget(D3DImage* image, const f32 color[4], const std::vector<D3D12_RECT>& rect)
{
    ASSERT(m_commandList, "nullptr");
    ASSERT(m_status == Status::ReadyToRecord, "not record");
    //TODO check color
    ID3D12GraphicsCommandList* cmdList = D3DGraphicsCommandList::getHandle();

    D3DGraphicsCommandList::transition(image, D3D12_RESOURCE_STATE_RENDER_TARGET);
    cmdList->ClearRenderTargetView(image->getDescriptorHandle(), color, static_cast<u32>(rect.size()), rect.data());
}

void D3DGraphicsCommandList::clearRenderTarget(D3DImage* image, f32 depth, u32 stencil, D3D12_CLEAR_FLAGS flags, const std::vector<D3D12_RECT>& rect)
{
    ASSERT(m_commandList, "nullptr");
    ASSERT(m_status == Status::ReadyToRecord, "not record");
    //TODO check depth
    D3DGraphicsCommandList::getHandle()->ClearDepthStencilView(image->getDescriptorHandle(), flags, depth, stencil, static_cast<u32>(rect.size()), rect.data());
}

void D3DGraphicsCommandList::setViewport(const std::vector<D3D12_VIEWPORT>& viewport)
{
    ASSERT(m_commandList, "nullptr");
    ASSERT(m_status == Status::ReadyToRecord, "not record");

    D3DGraphicsCommandList::getHandle()->RSSetViewports(static_cast<u32>(viewport.size()), viewport.data());
}

void D3DGraphicsCommandList::setScissor(const std::vector<D3D12_RECT>& scissors)
{
    ASSERT(m_commandList, "nullptr");
    ASSERT(m_status == Status::ReadyToRecord, "not record");

    D3DGraphicsCommandList::getHandle()->RSSetScissorRects(static_cast<u32>(scissors.size()), scissors.data());
}

} //namespace d3d12
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER
