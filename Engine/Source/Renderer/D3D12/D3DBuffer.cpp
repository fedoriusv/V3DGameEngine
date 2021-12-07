#include "D3DBuffer.h"
#include "Utils/Logger.h"

#ifdef D3D_RENDER
#include "D3DDebug.h"
#include "D3DGraphicContext.h"
#include "D3DCommandList.h"
#include "D3DCommandListManager.h"
#include "D3DMemoryHeap.h"
#include "D3DDeviceCaps.h"

namespace v3d
{
namespace renderer
{
namespace dx3d
{

D3DBuffer::D3DBuffer(ID3D12Device* device, Buffer::BufferType type, StreamBufferUsageFlags usageFlag, u64 size, const std::string& name, D3DMemoryHeapAllocator* allocator) noexcept
    : m_device(device)
    , m_allocator(nullptr)

    , m_resource(nullptr)

    , m_heapProperties(CD3DX12_HEAP_PROPERTIES())
    , m_state(D3D12_RESOURCE_STATE_COMMON)
    , m_type(type)
    , m_size(size)
#if D3D_DEBUG
    , m_debugName(name)
#endif
{
    LOG_DEBUG("D3DBuffer::D3DBuffer constructor %llx", this);

    switch (type)
    {
    case Buffer::BufferType::BufferType_VertexBuffer:
    {
        if (usageFlag & StreamBufferUsage::StreamBuffer_Dynamic)
        {
            m_heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
            m_state = D3D12_RESOURCE_STATE_GENERIC_READ;
        }
        else
        {
            m_heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
            m_state = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
        }

        break;
    }

    case Buffer::BufferType::BufferType_IndexBuffer:
    {
        if (usageFlag & StreamBufferUsage::StreamBuffer_Dynamic)
        {
            ASSERT(!(usageFlag & StreamBufferUsage::StreamBuffer_Write), "can't be readback");
            m_heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
            m_state = D3D12_RESOURCE_STATE_GENERIC_READ;
        }
        else
        {
            m_heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
            m_state = D3D12_RESOURCE_STATE_INDEX_BUFFER;
        }

        break;
    }

    case Buffer::BufferType::BufferType_UniformBuffer:
    {
        m_heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        m_state = D3D12_RESOURCE_STATE_GENERIC_READ;

        break;
    }

    case Buffer::BufferType::BufferType_StagingBuffer:
    {
        ASSERT(usageFlag & StreamBufferUsage::StreamBuffer_Read, "must be read");
        m_heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        m_state = D3D12_RESOURCE_STATE_GENERIC_READ;

        break;
    }

    default:
        ASSERT(false, "unsupported");
    }
}

D3DBuffer::~D3DBuffer()
{
    LOG_DEBUG("D3DBuffer::~D3DBuffer destructor %llx", this);
    ASSERT(!m_resource, "not nullptr");
}

bool D3DBuffer::create()
{
    if (m_resource)
    {
        ASSERT(false, "already created");
        return true;
    }

    D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(m_size);
    if (m_allocator)
    {
        //TODO
        //D3DMemoryHeap heap = m_allocator->acquireHeap();

        //HRESULT result = m_device->CreatePlacedResource(heap._heap, heap._offset, &resourceDesc, m_state, nullptr, DX_IID_PPV_ARGS(&m_resource));
        //if (FAILED(result))
        //{
        //    LOG_ERROR("D3DBuffer::create CreatePlacedResource is failed. Error %s", D3DDebug::stringError(result).c_str());
        //    return false;
        //}
    }
    else
    {
        HRESULT result = m_device->CreateCommittedResource(&m_heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, m_state, nullptr, DX_IID_PPV_ARGS(&m_resource));
        if (FAILED(result))
        {
            LOG_ERROR("D3DBuffer::create CreateCommittedResource is failed. Error %s", D3DDebug::stringError(result).c_str());
            return false;
        }
    }

#if D3D_DEBUG
    w16 wtext[64];
    if (!m_debugName.empty())
    {
        mbstowcs(wtext, m_debugName.c_str(), m_debugName.size() + 1);
    }
    else
    {
        std::string debugName = "BufferResource: " + std::to_string(reinterpret_cast<const u64>(this));
        mbstowcs(wtext, debugName.c_str(), debugName.size() + 1);
    }
    m_resource->SetName(LPCWSTR(wtext));
#endif

    return true;
}

void D3DBuffer::destroy()
{
    SAFE_DELETE(m_resource);
}

u64 D3DBuffer::getSize() const
{
    return m_size;
}

bool D3DBuffer::upload(Context* context, u32 offset, u64 size, const void* data)
{
    ASSERT(m_resource, "nullptr");
    if (m_heapProperties.IsCPUAccessible())
    {
        UINT8* bufferData = nullptr;
        CD3DX12_RANGE readRange = { offset, offset + size };
        HRESULT result = m_resource->Map(0, &readRange, reinterpret_cast<void**>(&bufferData));
        if (FAILED(result))
        {
            LOG_ERROR("D3DBuffer::upload map failed %s", D3DDebug::stringError(result).c_str());
            return false;
        }
        memcpy(bufferData, data, size);
        m_resource->Unmap(0, &readRange);
    }
    else
    {
        D3DGraphicContext* dxContext = static_cast<D3DGraphicContext*>(context);
        ASSERT(dxContext, "nullptr");

        D3DGraphicsCommandList* commandlist = static_cast<D3DGraphicsCommandList*>(dxContext->getOrAcquireCurrentCommandList());
        ASSERT(commandlist, "nullptr");

        D3DBuffer* copyBuffer = new D3DBuffer(m_device, BufferType::BufferType_StagingBuffer, StreamBufferUsage::StreamBuffer_Read, m_size, "UploadBufferResource", m_allocator);
        if (!copyBuffer->create())
        {
            copyBuffer->destroy();
            delete copyBuffer;

            ASSERT(false, "D3DBuffer::upload: Create copy buffer is failed");
            return false;
        }

        D3D12_RESOURCE_STATES oldState = getState();
        commandlist->transition(this, D3D12_RESOURCE_STATE_COPY_DEST, true);

        D3D12_PLACED_SUBRESOURCE_FOOTPRINT subResourceFootPrint = {};
        UINT subResourceNumRows = 0;
        UINT64 subResourceNumRowsSize = 0;
        UINT64 uploadBufferSize = 0;
        m_device->GetCopyableFootprints(&m_resource->GetDesc(), 0, 1, 0, &subResourceFootPrint, &subResourceNumRows, &subResourceNumRowsSize, &uploadBufferSize);
        ASSERT(uploadBufferSize == m_size, "must be same");

        D3D12_SUBRESOURCE_DATA subresourceData =
        {
            data,
            static_cast<LONG_PTR>(uploadBufferSize),
            static_cast<LONG_PTR>(uploadBufferSize)
        };

        if (UINT64 requiredSize = UpdateSubresources(commandlist->getHandle(), m_resource, copyBuffer->getResource(), 0, 1, uploadBufferSize, &subResourceFootPrint, &subResourceNumRows, &subResourceNumRowsSize, &subresourceData); requiredSize != m_size)
        {
            copyBuffer->destroy();
            delete copyBuffer;

            ASSERT(false, "copy is failed");
            return false;
        }
        commandlist->setUsed(copyBuffer, 0);

        commandlist->transition(this, oldState, true);

        if (D3DDeviceCaps::getInstance()->immediateSubmitUpload)
        {
            dxContext->submit(true);
        }

        dxContext->getResourceDeleter().requestToDelete(copyBuffer, [copyBuffer]() -> void
            {
                copyBuffer->destroy();
                delete copyBuffer;
            });
    }

    return true;
}

ID3D12Resource* D3DBuffer::getResource() const
{
    ASSERT(m_resource, "nullptr");
    return m_resource;
}

D3D12_GPU_VIRTUAL_ADDRESS D3DBuffer::getGPUAddress() const
{
    ASSERT(m_resource, "nullptr");
    return m_resource->GetGPUVirtualAddress();
}

D3D12_RESOURCE_STATES D3DBuffer::getState() const
{
    return m_state;
}

D3D12_RESOURCE_STATES D3DBuffer::setState(D3D12_RESOURCE_STATES state)
{
    return std::exchange(m_state, state);
}

} //namespace dx3d
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER
