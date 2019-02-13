#include "StreamBuffer.h"
#include "Renderer/Context.h"
#include "Renderer/Buffer.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace renderer
{
    /*CreateBufferCommand*/
class CreateBufferCommand : public Command
{
public:
    CreateBufferCommand(Buffer* buffer, u64 dataSize, void * data, bool shared) noexcept
        : m_buffer(buffer)
        , m_size(0)
        , m_data(nullptr)
        , m_shadred(shared)
    {
        LOG_DEBUG("CreateBufferCommand constructor");

        if (data && dataSize > 0)
        {
            m_size = dataSize;
            if (m_shadred)
            {
                m_data = data;
            }
            else
            {
                m_data = malloc(m_size); //TODO: get from pool
                memcpy(m_data, data, m_size);
            }
        }
    }

    ~CreateBufferCommand()
    {
        LOG_DEBUG("CreateBufferCommand destructor");

        if (m_data && m_shadred)
        {
            free(m_data); //TODO: return to pool
            m_data = nullptr;
        }
    }

    void execute(const renderer::CommandList& cmdList) override
    {
        LOG_DEBUG("CreateBufferCommand execute");
        if (!m_buffer->create())
        {
            m_buffer->notifyObservers();
            m_buffer->destroy();

            return;
        }

        if (m_data && m_size > 0)
        {
            m_buffer->upload(cmdList.getContext(), 0, m_size, m_data);
        }
    }

private:
    Buffer* m_buffer;
    u64     m_size;
    void*   m_data;
    bool    m_shadred;
};

    /*UpdateBufferCommand*/
class UpdateBufferCommand : public Command
{
public:
    UpdateBufferCommand(Buffer* buffer, u32 dataOffset, u64 dataSize, void * data, bool shared) noexcept
        : m_buffer(buffer)
        , m_offest(0)
        , m_size(0)
        , m_data(nullptr)
        , m_shadred(shared)
    {
        LOG_DEBUG("UpdateBufferCommand constructor");

        if (data && dataSize > 0)
        {
            m_offest = dataOffset;
            m_size = dataSize;
            if (m_shadred)
            {
                m_data = data;
            }
            else
            {
                m_data = malloc(m_size); //TODO: get from pool
                memcpy(m_data, data, m_size);
            }
        }
    }

    ~UpdateBufferCommand()
    {
        LOG_DEBUG("UpdateBufferCommand destructor");

        if (m_data && m_shadred)
        {
            free(m_data); //TODO: return to pool
            m_data = nullptr;
        }
    }

    void execute(const renderer::CommandList& cmdList) override
    {
        LOG_DEBUG("UpdateBufferCommand execute");
        if (m_data && m_size > 0)
        {
            m_buffer->upload(cmdList.getContext(), m_offest, m_size, m_data);
        }
    }

private:
    Buffer* m_buffer;
    u32     m_offest;
    u64     m_size;
    void*   m_data;
    bool    m_shadred;
};

    /*DestroyBufferCommand*/
class DestroyBufferCommand : public Command
{
public:
    DestroyBufferCommand(Buffer* buffer) noexcept
        : m_buffer(buffer)
    {
        LOG_DEBUG("DestroyBufferCommand constructor");
    }

    ~DestroyBufferCommand()
    {
        LOG_DEBUG("DestroyBufferCommand destructor");
    }

    void execute(const renderer::CommandList& cmdList) override
    {
        LOG_DEBUG("DestroyBufferCommand execute");
        cmdList.getContext()->removeBuffer(m_buffer);
    }

private:
    Buffer* m_buffer;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////

VertexStreamBuffer::VertexStreamBuffer(renderer::CommandList& cmdList, StreamBufferUsageFlags usage, u64 size, const u8* data) noexcept
    : m_cmdList(cmdList)
    , m_data(nullptr)
    , m_size(size)
    , m_buffer(nullptr)

    , m_usage(usage)
{
    if (data && size > 0)
    {
        m_data = malloc(size);
        memcpy(m_data, data, size);
    }

    m_buffer = m_cmdList.getContext()->createBuffer(Buffer::BufferType::BufferType_VertexBuffer, m_usage, m_size);
    ASSERT(m_buffer, "m_buffer is nullptr");

    if (m_cmdList.isImmediate())
    {
        if (!m_buffer->create())
        {
            m_buffer->destroy();
            delete m_buffer;
            m_buffer = nullptr;

            return;
        }
        m_buffer->registerNotify(this);

        if (m_data)
        {
            m_buffer->upload(m_cmdList.getContext(), 0, m_size, m_data);
        }
    }
    else
    {
        m_buffer->registerNotify(this);
        m_cmdList.pushCommand(new CreateBufferCommand(m_buffer, m_size, m_data, (m_usage & StreamBuffer_Shared)));
    }
}

VertexStreamBuffer::~VertexStreamBuffer()
{
    ASSERT(m_buffer, "buffer nullptr");
    if (m_cmdList.isImmediate())
    {
        m_cmdList.getContext()->removeBuffer(m_buffer);
    }
    else
    {
        m_cmdList.pushCommand(new DestroyBufferCommand(m_buffer));
    }

    if (m_data)
    {
        free(m_data);
        m_data = nullptr;
    }
}

void VertexStreamBuffer::handleNotify(utils::Observable * ob)
{
    LOG_DEBUG("VertexStreamBuffer::handleNotify to delete buffer %xll", this);
    ASSERT(m_buffer == ob, "not same");

    //delete m_buffer;
    m_buffer = nullptr;
}

bool VertexStreamBuffer::update(u32 offset, u64 size, const u8* data)
{
    if (!m_buffer)
    {
        ASSERT(false, "buffer nullptr");
        return false;
    }

    if (size > 0 && data)
    {
        if (m_size != size && offset == 0)
        {
            if (!(m_usage & StreamBuffer_Dynamic) || (m_usage & StreamBuffer_Shared))
            {
                ASSERT(false, "static buffer");
                return false;
            }

            if (m_data)
            {
                delete m_data;
            }
            m_data = malloc(size);
            m_size = size;
        }

        memcpy(m_data, data, size);

        if (m_cmdList.isImmediate())
        {
            return m_buffer->upload(m_cmdList.getContext(), offset, m_size, m_data);
        }
        else
        {
            m_cmdList.pushCommand(new UpdateBufferCommand(m_buffer, offset, m_size, m_data, (m_usage & StreamBuffer_Shared)));
            return true;
        }
    }

    return false;
}

bool VertexStreamBuffer::read(u32 offset, u64 size, u8 * data)
{
    if (!m_buffer)
    {
        ASSERT(false, "buffer nullptr");
        return false;
    }

    ASSERT(false, "not implemented");
    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

IndexStreamBuffer::IndexStreamBuffer(CommandList & cmdList, StreamBufferUsageFlags usage, StreamIndexBufferType type, u32 count, const u8 * data) noexcept
    : m_cmdList(cmdList)
    , m_type(type)
    , m_count(count)
    , m_buffer(nullptr)

    , m_usage(usage)
{
    u32 size = count * (type == StreamIndexBufferType::IndexType_16) ? sizeof(16) : sizeof(32);
    if (data && size > 0)
    {
        m_data = malloc(size);
        memcpy(m_data, data, size);
    }

    m_buffer = m_cmdList.getContext()->createBuffer(Buffer::BufferType::BufferType_IndexBuffer, m_usage, size);
    ASSERT(m_buffer, "m_buffer is nullptr");

    if (m_cmdList.isImmediate())
    {
        if (!m_buffer->create())
        {
            m_buffer->destroy();
            delete m_buffer;
            m_buffer = nullptr;

            return;
        }
        m_buffer->registerNotify(this);

        if (m_data)
        {
            m_buffer->upload(m_cmdList.getContext(), 0, size, m_data);
        }
    }
    else
    {
        m_buffer->registerNotify(this);
        m_cmdList.pushCommand(new CreateBufferCommand(m_buffer, size, m_data, (m_usage & StreamBuffer_Shared)));
    }
}

IndexStreamBuffer::~IndexStreamBuffer()
{
    ASSERT(m_buffer, "buffer nullptr");
    if (m_cmdList.isImmediate())
    {
        m_cmdList.getContext()->removeBuffer(m_buffer);
    }
    else
    {
        m_cmdList.pushCommand(new DestroyBufferCommand(m_buffer));
    }

    if (m_data)
    {
        free(m_data);
        m_data = nullptr;
    }
}

} //namespace renderer
} //namespace v3d
