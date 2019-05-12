#include "StreamBuffer.h"
#include "Renderer/Context.h"
#include "Renderer/Buffer.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace renderer
{
    /*CommandCreateBuffer*/
class CommandCreateBuffer : public Command
{
public:
    CommandCreateBuffer(Buffer* buffer, u64 dataSize, void * data, bool shared) noexcept
        : m_buffer(buffer)
        , m_size(0)
        , m_data(nullptr)
        , m_shadred(shared)
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandCreateBuffer constructor");
#endif //DEBUG_COMMAND_LIST
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

    ~CommandCreateBuffer()
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandCreateBuffer destructor");
#endif //DEBUG_COMMAND_LIST
        if (m_data && !m_shadred)
        {
            free(m_data); //TODO: return to pool
            m_data = nullptr;
        }
    }

    void execute(const renderer::CommandList& cmdList) override
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandCreateBuffer execute");
#endif //DEBUG_COMMAND_LIST
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

    /*CommandUpdateBuffer*/
class CommandUpdateBuffer : public Command
{
public:
    CommandUpdateBuffer(Buffer* buffer, u32 dataOffset, u64 dataSize, void * data, bool shared) noexcept
        : m_buffer(buffer)
        , m_offest(0)
        , m_size(0)
        , m_data(nullptr)
        , m_shadred(shared)
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandUpdateBuffer constructor");
#endif //DEBUG_COMMAND_LIST

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

    ~CommandUpdateBuffer()
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandUpdateBuffer destructor");
#endif //DEBUG_COMMAND_LIST
        if (m_data && !m_shadred)
        {
            free(m_data); //TODO: return to pool
            m_data = nullptr;
        }
    }

    void execute(const renderer::CommandList& cmdList) override
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandUpdateBuffer execute");
#endif //DEBUG_COMMAND_LIST
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

    /*CommandDestroyBuffer*/
class CommandDestroyBuffer : public Command
{
public:
    CommandDestroyBuffer(Buffer* buffer) noexcept
        : m_buffer(buffer)
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandDestroyBuffer constructor");
#endif //DEBUG_COMMAND_LIST
    }

    ~CommandDestroyBuffer()
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandDestroyBuffer destructor");
#endif //DEBUG_COMMAND_LIST
    }

    void execute(const renderer::CommandList& cmdList) override
    {
#if DEBUG_COMMAND_LIST
        LOG_DEBUG("CommandDestroyBuffer execute");
#endif //DEBUG_COMMAND_LIST
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
        m_cmdList.pushCommand(new CommandCreateBuffer(m_buffer, m_size, m_data, (m_usage & StreamBuffer_Shared)));
    }
}

VertexStreamBuffer::~VertexStreamBuffer()
{
    ASSERT(m_buffer, "buffer nullptr");
    m_buffer->unregisterNotify(this);

    if (m_cmdList.isImmediate())
    {
        m_cmdList.getContext()->removeBuffer(m_buffer);
    }
    else
    {
        m_cmdList.pushCommand(new CommandDestroyBuffer(m_buffer));
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
                free(m_data);
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
            m_cmdList.pushCommand(new CommandUpdateBuffer(m_buffer, offset, m_size, m_data, (m_usage & StreamBuffer_Shared)));
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
    , m_data(nullptr)

    , m_usage(usage)
{
    u32 size = count * ((type == StreamIndexBufferType::IndexType_16) ? sizeof(16) : sizeof(32));
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
        m_cmdList.pushCommand(new CommandCreateBuffer(m_buffer, size, m_data, (m_usage & StreamBuffer_Shared)));
    }
}

IndexStreamBuffer::~IndexStreamBuffer()
{
    ASSERT(m_buffer, "buffer nullptr");
    m_buffer->unregisterNotify(this);

    if (m_cmdList.isImmediate())
    {
        m_cmdList.getContext()->removeBuffer(m_buffer);
    }
    else
    {
        m_cmdList.pushCommand(new CommandDestroyBuffer(m_buffer));
    }

    if (m_data)
    {
        free(m_data);
        m_data = nullptr;
    }
}

void IndexStreamBuffer::handleNotify(utils::Observable* ob)
{
    LOG_DEBUG("IndexStreamBuffer::handleNotify to delete buffer %xll", this);
    ASSERT(m_buffer == ob, "not same");

    m_buffer = nullptr;
}

u32 IndexStreamBuffer::getIndexCount() const
{
    return m_count;
}

StreamIndexBufferType IndexStreamBuffer::getIndexBufferType() const
{
    return m_type;
}

} //namespace renderer
} //namespace v3d
