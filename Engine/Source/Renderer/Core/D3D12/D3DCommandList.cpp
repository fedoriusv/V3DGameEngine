#include "D3DCommandList.h"
#include "Utils/Logger.h"

#ifdef D3D_RENDER
#include "D3DResource.h"
#include "D3DDebug.h"
#include "D3DImage.h"
#include "D3DBuffer.h"
#include "D3DFence.h"
#include "D3DGraphicPipelineState.h"
#include "D3DComputePipelineState.h"
#include "D3DGraphicContext.h"
#include "D3DRenderTarget.h"
#include "D3DDeviceCaps.h"


namespace v3d
{
namespace renderer
{
namespace dx3d
{

D3D12_COMMAND_LIST_TYPE D3DCommandList::convertCommandListTypeToD3DType(D3DCommandList::Type type)
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
    if (m_commandList)
    {
        SAFE_DELETE(m_commandList);
    }

    if (m_commandAllocator)
    {
        if (m_ownAllocator)
        {
            m_commandAllocator->Reset();
            SAFE_DELETE(m_commandAllocator);
        }
        m_commandAllocator = nullptr;
    }

    if (m_fence)
    {
        delete m_fence;
        m_fence = nullptr;
    }
}

void D3DCommandList::setUsed(D3DResource* resource, u64 frame)
{
    resource->attachFence(m_fence, frame);
    m_resources.push_back(resource);
}

D3DCommandList::D3DCommandList(ID3D12Device* device, Type type) noexcept
    : m_device(device)
    , m_commandList(nullptr)

    , m_commandAllocator(nullptr)
    , m_ownAllocator(true)

    , m_status(Status::Initial)
    , m_type(type)

    , m_fence(new D3DFence(device, 1U))
{
    LOG_DEBUG("D3DCommandList::D3DCommandList constructor %llx", this);
}

D3DCommandList::~D3DCommandList()
{
    LOG_DEBUG("D3DCommandList::~D3DCommandList destructor %llx", this);
    ASSERT(m_status == Status::Finish, "not finished");

    ASSERT(!m_commandList, "not nullptr");
    ASSERT(!m_commandAllocator, "not nullptr");

    ASSERT(m_resources.empty(), "not empty");
}

ID3D12CommandList* D3DCommandList::getHandle() const
{
    ASSERT(m_commandList, "nullptr");
    return m_commandList;
}

void D3DCommandList::init(ID3D12CommandList* cmdList, ID3D12CommandAllocator* allocator, bool own)
{
    m_commandAllocator = allocator;
    m_ownAllocator = own;

    m_commandList = cmdList;
}

bool D3DCommandList::checkOnComplete()
{
    if (m_fence->completed())
    {
        for (D3DResource* resource : m_resources)
        {
            resource->detachFence(m_fence);
        }
        m_resources.clear();

        return true;
    }

    return false;
}

D3DGraphicsCommandList::D3DGraphicsCommandList(ID3D12Device* device, Type type) noexcept
    : D3DCommandList(device, type)
{
    LOG_DEBUG("D3DGraphicsCommandList::D3DGraphicsCommandList constructor %llx", this);
}

D3DGraphicsCommandList::~D3DGraphicsCommandList()
{
    LOG_DEBUG("D3DGraphicsCommandList::~D3DGraphicsCommandList destructor %llx", this);
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
        HRESULT result = D3DGraphicsCommandList::getHandle()->Reset(m_commandAllocator, nullptr);
        ASSERT(SUCCEEDED(result), "error");
    }

    m_status = Status::ReadyToRecord;
}

void D3DGraphicsCommandList::close()
{
    ASSERT(m_commandList, "nullptr");

    m_transition.execute(this);

    HRESULT result = D3DGraphicsCommandList::getHandle()->Close();
    ASSERT(SUCCEEDED(result), "error");

    m_status = Status::Closed;
}

void D3DGraphicsCommandList::setRenderTarget(D3DRenderTarget* target)
{
    ASSERT(m_commandList, "nullptr");
    ASSERT(m_status == Status::ReadyToRecord, "not record");

    ID3D12GraphicsCommandList* cmdList = D3DGraphicsCommandList::getHandle();
    if (target->getDescription()._hasDepthStencilAttahment)
    {
        cmdList->OMSetRenderTargets(static_cast<UINT>(target->getColorDescHandles().size()), target->getColorDescHandles().data(), true, &target->getDepthStencilDescHandles());
    }
    else
    {
        cmdList->OMSetRenderTargets(static_cast<UINT>(target->getColorDescHandles().size()), target->getColorDescHandles().data(), true, nullptr);
    }

    this->setUsed(target, 0);
}

void D3DGraphicsCommandList::setPipelineState(D3DGraphicPipelineState* pipeline)
{
    ASSERT(m_commandList, "nullptr");
    ASSERT(m_status == Status::ReadyToRecord, "not record");

    ID3D12GraphicsCommandList* cmdList = D3DGraphicsCommandList::getHandle();
    cmdList->SetPipelineState(pipeline->getHandle());

    cmdList->IASetPrimitiveTopology(pipeline->getTopology());
    cmdList->SetGraphicsRootSignature(pipeline->getSignatureHandle());

    this->setUsed(pipeline, 0);
}

void D3DGraphicsCommandList::setPipelineState(D3DComputePipelineState* pipeline)
{
    ASSERT(m_commandList, "nullptr");
    ASSERT(m_status == Status::ReadyToRecord, "not record");

    ID3D12GraphicsCommandList* cmdList = D3DGraphicsCommandList::getHandle();
    cmdList->SetPipelineState(pipeline->getHandle());
    cmdList->SetComputeRootSignature(pipeline->getSignatureHandle());

    this->setUsed(pipeline, 0);
}

void D3DGraphicsCommandList::setDescriptorTables(const std::vector<ID3D12DescriptorHeap*>& heaps, const std::map<u32, std::tuple<D3DDescriptorHeap*, u32>>& desc, Pipeline::PipelineType type)
{
    ASSERT(m_commandList, "nullptr");
    ASSERT(m_status == Status::ReadyToRecord, "not record");

    m_transition.execute(this);

    ID3D12GraphicsCommandList* cmdList = D3DGraphicsCommandList::getHandle();

    cmdList->SetDescriptorHeaps(static_cast<UINT>(heaps.size()), heaps.data());
    for (auto& [paramIndex, descHeap] : desc)
    {
        auto& [heap, index] = descHeap;
        CD3DX12_GPU_DESCRIPTOR_HANDLE dtHandle(heap->getGPUHandle(), index, heap->getIncrement());
        if (type == Pipeline::PipelineType::PipelineType_Graphic)
        {
            cmdList->SetGraphicsRootDescriptorTable(paramIndex, dtHandle);
        }
        else
        {
            ASSERT(type == Pipeline::PipelineType::PipelineType_Compute, "wrong type");
            cmdList->SetComputeRootDescriptorTable(paramIndex, dtHandle);
        }
    }
}

void D3DGraphicsCommandList::setVertexState(u32 startSlot, const std::vector<u32>& strides, const std::vector<Buffer*>& buffers)
{
    ASSERT(m_commandList, "nullptr");
    ASSERT(m_status == Status::ReadyToRecord, "not record");

    std::vector<D3D12_VERTEX_BUFFER_VIEW> dxBuffers(buffers.size());
    for (u32 index = 0; index < buffers.size(); ++index)
    {
        D3DBuffer* dxBuffer = static_cast<D3DBuffer*>(buffers[index]);
        this->setUsed(dxBuffer, 0);

        D3D12_VERTEX_BUFFER_VIEW& view = dxBuffers[index];
        view.BufferLocation = dxBuffer->getGPUAddress();
        view.SizeInBytes = static_cast<UINT>(dxBuffer->getSize());
        view.StrideInBytes = strides[index];
    }

    ID3D12GraphicsCommandList* cmdList = D3DGraphicsCommandList::getHandle();
    cmdList->IASetVertexBuffers(startSlot, static_cast<u32>(dxBuffers.size()), dxBuffers.data());
}

void D3DGraphicsCommandList::setIndexState(Buffer* buffer, DXGI_FORMAT format)
{
    ASSERT(m_commandList, "nullptr");
    ASSERT(m_status == Status::ReadyToRecord, "not record");

    D3DBuffer* dxBuffer = static_cast<D3DBuffer*>(buffer);
    this->setUsed(dxBuffer, 0);

    D3D12_INDEX_BUFFER_VIEW bufferView = {};
    bufferView.BufferLocation = dxBuffer->getGPUAddress();
    bufferView.SizeInBytes = static_cast<UINT>(dxBuffer->getSize());
    bufferView.Format = format;

    ID3D12GraphicsCommandList* cmdList = D3DGraphicsCommandList::getHandle();
    cmdList->IASetIndexBuffer(&bufferView);
}

void D3DGraphicsCommandList::transition(D3DImage* image, const Image::Subresource& subresource, D3D12_RESOURCE_STATES states, bool immediateTransition)
{
    ASSERT(m_commandList, "nullptr");
    ASSERT(m_status == Status::ReadyToRecord, "not record");
    this->setUsed(image, 0);

    for (u32 layer = 0; layer < subresource._layers; ++layer)
    {
        for (u32 mip = 0; mip < subresource._mips; ++mip)
        {
            if (image->getState(Image::makeImageSubresource(subresource._baseLayer + layer, 1, subresource._baseMip + mip, 1)) != states)
            {
                D3D12_RESOURCE_STATES oldState = image->setState(Image::makeImageSubresource(subresource._baseLayer + layer, 1, subresource._baseMip + mip, 1), states);
                u32 index = D3D12CalcSubresource(subresource._baseLayer + layer, subresource._baseMip + mip, 0, 1, 1);

                D3D12_RESOURCE_BARRIER resourceBarrierDesc = {};
                resourceBarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                resourceBarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                resourceBarrierDesc.Transition.pResource = image->getResource();
                resourceBarrierDesc.Transition.StateBefore = oldState;
                resourceBarrierDesc.Transition.StateAfter = states;
                resourceBarrierDesc.Transition.Subresource = index;

                m_transition.add(resourceBarrierDesc);
            }
        }
    }

    if (D3DDeviceCaps::getInstance()->immediateTransition || immediateTransition)
    {
        m_transition.execute(this);
    }
}

ID3D12GraphicsCommandList1* D3DGraphicsCommandList::getHandle() const
{
    ASSERT(m_commandList, "nullptr");
    return static_cast<ID3D12GraphicsCommandList1*>(m_commandList);
}

void D3DGraphicsCommandList::clearRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView, const f32 color[4], const std::vector<D3D12_RECT>& rect)
{
    ASSERT(m_commandList, "nullptr");
    ASSERT(m_status == Status::ReadyToRecord, "not record");

    m_transition.execute(this);

    ID3D12GraphicsCommandList* cmdList = D3DGraphicsCommandList::getHandle();
    cmdList->ClearRenderTargetView(renderTargetView, color, static_cast<u32>(rect.size()), rect.data());
}

void D3DGraphicsCommandList::clearRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView, f32 depth, u32 stencil, D3D12_CLEAR_FLAGS flags, const std::vector<D3D12_RECT>& rect)
{
    ASSERT(m_commandList, "nullptr");
    ASSERT(m_status == Status::ReadyToRecord, "not record");

    m_transition.execute(this);

    ID3D12GraphicsCommandList* cmdList = D3DGraphicsCommandList::getHandle();
    cmdList->ClearDepthStencilView(renderTargetView, flags, depth, stencil, static_cast<u32>(rect.size()), rect.data());
}

void D3DGraphicsCommandList::draw(u32 vertexCountPerInstance, u32 instanceCount, u32 startVertexLocation, u32 startInstanceLocation)
{
    ASSERT(m_commandList, "nullptr");
    ASSERT(m_status == Status::ReadyToRecord, "not record");

    m_transition.execute(this);

    ID3D12GraphicsCommandList* cmdList = D3DGraphicsCommandList::getHandle();
    cmdList->DrawInstanced(vertexCountPerInstance, instanceCount, startVertexLocation, startInstanceLocation);
}

void D3DGraphicsCommandList::drawIndexed(u32 indexCountPerInstance, u32 instanceCount, u32 startIndexLocation, u32 baseVertexLocation, u32 startInstanceLocation)
{
    ASSERT(m_commandList, "nullptr");
    ASSERT(m_status == Status::ReadyToRecord, "not record");

    m_transition.execute(this);

    ID3D12GraphicsCommandList* cmdList = D3DGraphicsCommandList::getHandle();
    cmdList->DrawIndexedInstanced( indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
}

void D3DGraphicsCommandList::dispatch(const core::Dimension3D& dimension)
{
    ASSERT(m_commandList, "nullptr");
    ASSERT(m_status == Status::ReadyToRecord, "not record");

    m_transition.execute(this);

    ID3D12GraphicsCommandList* cmdList = D3DGraphicsCommandList::getHandle();
    cmdList->Dispatch(dimension.width, dimension.height, dimension.depth);
}

void D3DGraphicsCommandList::setViewport(const std::vector<D3D12_VIEWPORT>& viewport)
{
    ASSERT(m_commandList, "nullptr");
    ASSERT(m_status == Status::ReadyToRecord, "not record");

    ID3D12GraphicsCommandList* cmdList = D3DGraphicsCommandList::getHandle();
    cmdList->RSSetViewports(static_cast<u32>(viewport.size()), viewport.data());
}

void D3DGraphicsCommandList::setScissor(const std::vector<D3D12_RECT>& scissors)
{
    ASSERT(m_commandList, "nullptr");
    ASSERT(m_status == Status::ReadyToRecord, "not record");

    ID3D12GraphicsCommandList* cmdList = D3DGraphicsCommandList::getHandle();
    cmdList->RSSetScissorRects(static_cast<u32>(scissors.size()), scissors.data());
}

void D3DGraphicsCommandList::resolve(D3DImage* image, const Image::Subresource& subresource, D3DImage* resolvedImage, const Image::Subresource& resolvedSubresource)
{
    ASSERT(m_commandList, "nullptr");
    ASSERT(m_status == Status::ReadyToRecord, "not record");
    ASSERT(image && resolvedImage, "nullptr");

    m_transition.execute(this);

    u32 subresourceIndex = D3D12CalcSubresource(subresource._baseLayer, subresource._baseMip, 0, subresource._mips, subresource._layers);
    u32 resolvedSubresourceIndex = D3D12CalcSubresource(resolvedSubresource._baseLayer, resolvedSubresource._baseMip, 0, resolvedSubresource._mips, resolvedSubresource._layers);

    ID3D12GraphicsCommandList* cmdList = D3DGraphicsCommandList::getHandle();
    cmdList->ResolveSubresource(resolvedImage->getResource(), resolvedSubresourceIndex, image->getResource(), subresourceIndex, D3DImage::getResolveCompatibilityFormat(resolvedImage->getFormat()));

    this->setUsed(image, 0);
    this->setUsed(resolvedImage, 0);
}

void D3DGraphicsCommandList::setViewInstanceMask(u32 mask)
{
    ASSERT(m_commandList, "nullptr");
    ASSERT(m_status == Status::ReadyToRecord, "not record");

    ID3D12GraphicsCommandList1* cmdList = D3DGraphicsCommandList::getHandle();
    cmdList->SetViewInstanceMask(mask);
}

void D3DGraphicsCommandList::BarrierResources::add(const D3D12_RESOURCE_BARRIER& resource)
{
    _barrierResources.push_back(resource);
}

void D3DGraphicsCommandList::BarrierResources::execute(D3DGraphicsCommandList* commandList)
{
    ASSERT(commandList && commandList->m_commandList, "nullptr");
    ASSERT(commandList->m_status == Status::ReadyToRecord, "not record");

    if (!_barrierResources.empty())
    {
        ID3D12GraphicsCommandList* cmdList = commandList->getHandle();
        cmdList->ResourceBarrier(static_cast<UINT>(_barrierResources.size()), _barrierResources.data());

        _barrierResources.clear();
    }
}

} //namespace dx3d
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER