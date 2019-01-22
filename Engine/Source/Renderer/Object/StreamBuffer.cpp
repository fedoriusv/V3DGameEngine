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
    {
        LOG_DEBUG("CreateBufferCommand constructor");

        if (data && dataSize > 0)
        {
            m_size = dataSize;
            if (shared)
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

        if (m_data)
        {
            free(m_data); //TODO: return to pool
            m_data = nullptr;
        }
    }

    void execute(const renderer::CommandList& cmdList) override
    {
        if (!m_buffer->create())
        {
            m_buffer->notifyObservers();

            m_buffer->destroy();
            delete m_buffer;

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
};

/////////////////////////////////////////////////////////////////////////////////////////////////////

VertexStreamBuffer::VertexStreamBuffer(renderer::CommandList& cmdList, u16 usageFlag, u64 size, void* data) noexcept
    : m_cmdList(cmdList)
    , m_buffer(nullptr)
{
    m_buffer = m_cmdList.getContext()->createBuffer(Buffer::BufferType::BufferType_VertexBuffer, usageFlag, size);
    ASSERT(m_buffer, "m_buffer is nullptr");

    m_lock = true;
    if (m_cmdList.isImmediate())
    {
        if (!m_buffer->create())
        {
            m_buffer->destroy();

            delete m_buffer;
            m_buffer = nullptr;
            m_lock = false;
        }
        m_buffer->registerNotify(this);

        if (data)
        {
            m_buffer->upload(m_cmdList.getContext(), 0, size, data);
        }
    }
    else
    {
        m_buffer->registerNotify(this);
        m_cmdList.pushCommand(new CreateBufferCommand(m_buffer, size, data, true));
    }
}

VertexStreamBuffer::~VertexStreamBuffer()
{
    //TODO
}

bool VertexStreamBuffer::isLocked() const
{
    return m_lock;
}

void VertexStreamBuffer::handleNotify(utils::Observable * ob)
{
}

void VertexStreamBuffer::update(u32 stream, u32 size, void * data)
{
    //TODO:
}


} //namespace renderer
} //namespace v3d
