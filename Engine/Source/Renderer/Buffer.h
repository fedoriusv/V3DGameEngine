#pragma once

#include "Object.h"
#include "Render.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Device;
    class RenderBuffer;

    struct GeometryBufferDesc;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Buffer base class. Game side. Wrapper on RenderBuffer object
    */
    class Buffer : public Object
    {
    protected:

        /**
        * @brief Buffer constructor
        * @param CommandList& cmdList [required]
        * @param  StreamBufferUsageFlags usage [required]
        * @param const std::string& name [optional]
        */
        explicit Buffer(BufferUsageFlags usage) noexcept;

        /**
        * @brief Buffer destructor
        */
        virtual ~Buffer();

        bool isUsageFlagContains(BufferUsageFlags usage) const;

        RenderBuffer*       m_buffer;
        BufferUsageFlags    m_usage;

        friend GeometryBufferDesc;

    };

    inline bool Buffer::isUsageFlagContains(BufferUsageFlags usage) const
    {
        return m_usage & usage;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VertexBuffer class. Game side
    */
    class VertexBuffer : public Buffer
    {
    public:

        /**
        * @brief getSize.
        */
        u64 getSize() const;

    private:

        /**
        * @brief VertexStreamBuffer constructor. Used to create buffer data objects.
        * @param StreamBufferUsageFlags usage [required]
        * @param u64 size [required] in bytes
        * @param  const void* data [required]
        * @param const std::string& name [optional]
        */
        explicit VertexBuffer(Device* device, BufferUsageFlags usage, u64 size, const std::string& name = "") noexcept;

        /**
        * @brief VertexBuffer destructor
        */
        ~VertexBuffer();

    private:

        VertexBuffer(const VertexBuffer&) = delete;
        VertexBuffer& operator=(const VertexBuffer&) = delete;

        const u64 m_size;
    };

    inline u64 VertexBuffer::getSize() const
    {
        return m_size;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief IndexBuffer class. Game side
    */
    class IndexBuffer : public Buffer
    {
    public:

        /**
        * @brief getIndexCount gets count of indexes.
        * @return Count of indexes
        */
        u32 getIndexCount() const;

        /**
        * @brief getIndexBufferType gets type of indexes.
        * @return type of indexes StreamIndexBufferType
        */
        IndexBufferType getIndexBufferType() const;

    private:

        /**
        * @brief IndexStreamBuffer constructor. Used to create buffer index objects.
        * @param StreamBufferUsageFlags usage [required]
        * @param StreamIndexBufferType type [required]
        * @param u32 count [required]
        * @param  const void* data [required]
        * @param const std::string& name [optional]
        */
        explicit IndexBuffer(BufferUsageFlags usage, IndexBufferType type, u32 count, const std::string& name = "") noexcept;

        /**
        * @brief IndexStreamBuffer destructor
        */
        ~IndexBuffer();

    private:

        IndexBuffer(const IndexBuffer&) = delete;
        IndexBuffer& operator=(const IndexBuffer&) = delete;

        IndexBufferType m_type;
        const u32       m_count;
    };

    inline u32 IndexBuffer::getIndexCount() const
    {
        return m_count;
    }

    inline IndexBufferType IndexBuffer::getIndexBufferType() const
    {
        return m_type;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief GeometryBufferDesc struct
    */
    struct GeometryBufferDesc
    {
        GeometryBufferDesc() noexcept
            : _indexBuffer(nullptr)
            , _indexOffset(0U)
            , _indexType(IndexBufferType::IndexType_16)
        {
        }

        GeometryBufferDesc(const VertexBuffer* vertex, u32 stream, u64 stride) noexcept
            : _indexBuffer(nullptr)
            , _indexOffset(0U)
            , _indexType(IndexBufferType::IndexType_16)

            , _vertexBuffers({ vertex->m_buffer })
            , _streamIDs({ stream })
            , _strides({ stride })
        {
            _offsets.resize(_vertexBuffers.size(), 0);
        }

        GeometryBufferDesc(const VertexBuffer* vertex, u32 stream, u64 stride, u64 offset) noexcept
            : _indexBuffer(nullptr)
            , _indexOffset(0U)
            , _indexType(IndexBufferType::IndexType_16)

            , _vertexBuffers({ vertex->m_buffer })
            , _streamIDs({ stream })
            , _strides({ stride })
            , _offsets({ offset })
        {
        }

        GeometryBufferDesc(const IndexBuffer* index, u32 indexOffset, const VertexBuffer* vertex, u32 stream, u64 stride, u64 vertexOffset) noexcept
            : _indexBuffer(index->m_buffer)
            , _indexOffset(indexOffset)
            , _indexType(IndexBufferType::IndexType_16)

            , _vertexBuffers({ vertex->m_buffer })
            , _streamIDs({ stream })
            , _strides({ stride })
            , _offsets({ vertexOffset })
        {
        }

        ~GeometryBufferDesc() = default;

        RenderBuffer*               _indexBuffer;
        u64                         _indexOffset;
        IndexBufferType             _indexType;

        std::vector<RenderBuffer*>  _vertexBuffers;
        std::vector<u32>            _streamIDs;
        std::vector<u64>            _offsets;
        std::vector<u64>            _strides;

    };

    ///////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
