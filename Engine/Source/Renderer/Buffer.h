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
    public:

        /**
        * @brief getBufferHandle method
        * @return BufferHandle of texture
        */
        BufferHandle getBufferHandle() const;

        /**
        * @brief hasUsageFlag method
        * @return true if flag contains
        */
        bool hasUsageFlag(BufferUsage usage) const;

        /**
        * @brief map method
        * @return void*
        */
        template<typename Type = void*>
        Type* map(u32 offset = ~1, u32 size = ~1)
        {
            return static_cast<Type*>(map_Impl(offset, size));
        }

        /**
        * @brief unmap method
        */
        void unmap(u32 offset = ~1, u32 size = ~1);

    protected:

        /**
        * @brief Buffer constructor
        * @param BufferUsageFlags usage [required]
        */
        explicit Buffer(BufferUsageFlags usage) noexcept;

        /**
        * @brief Buffer destructor
        */
        virtual ~Buffer();

        BufferHandle     m_buffer;
        BufferUsageFlags m_usage;

        friend GeometryBufferDesc;

    private:

        void* map_Impl(u32 offset, u32 size);
    };

    inline bool Buffer::hasUsageFlag(BufferUsage usage) const
    {
        return m_usage & usage;
    }

    inline BufferHandle Buffer::getBufferHandle() const
    {
        return m_buffer;
    }



    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VertexBuffer class. Game side
    */
    class VertexBuffer : public Buffer
    {
    public:

        /**
        * @brief getSize. Buffer size in bytes
        * @return Count of indexes
        */
        u32 getSize() const;

        /**
        * @brief getVertices. Count of vertices
        * @return Count of vertices
        */
        u32 getVerticesCount() const;

    public:

        /**
        * @brief VertexBuffer constructor. Used to create buffer data objects.
        * @param BufferUsageFlags usage [required]
        * @param u32 count [required]
        * @param u32 size [required] in bytes
        * @param const std::string& name [optional]
        */
        explicit VertexBuffer(Device* device, BufferUsageFlags usage, u32 count, u32 size, const std::string& name = "") noexcept;

        /**
        * @brief VertexBuffer destructor
        */
        ~VertexBuffer();

    private:

        VertexBuffer(const VertexBuffer&) = delete;
        VertexBuffer& operator=(const VertexBuffer&) = delete;

        Device* const m_device;
        const u32     m_size;
        const u32     m_count;
    };

    inline u32 VertexBuffer::getSize() const
    {
        return m_size;
    }

    inline u32 VertexBuffer::getVerticesCount() const
    {
        return m_count;
    }


    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief IndexBuffer class. Game side
    */
    class IndexBuffer : public Buffer
    {
    public:

        /**
        * @brief getIndicesCount gets count of indices.
        * @return Count of indices
        */
        u32 getIndicesCount() const;

        /**
        * @brief getIndexBufferType gets type of index buffer.
        * @return IndexBufferType type
        */
        IndexBufferType getIndexBufferType() const;

    public:

        /**
        * @brief IndexBuffer constructor. Used to create buffer index objects.
        * @param BufferUsageFlags usage [required]
        * @param IndexBufferType type [required]
        * @param u32 count [required]
        * @param const std::string& name [optional]
        */
        explicit IndexBuffer(Device* device, BufferUsageFlags usage, IndexBufferType type, u32 count, const std::string& name = "") noexcept;

        /**
        * @brief IndexBuffer destructor
        */
        ~IndexBuffer();

    private:

        IndexBuffer(const IndexBuffer&) = delete;
        IndexBuffer& operator=(const IndexBuffer&) = delete;

        Device* const   m_device;
        IndexBufferType m_type;
        const u32       m_count;
    };

    inline u32 IndexBuffer::getIndicesCount() const
    {
        return m_count;
    }

    inline IndexBufferType IndexBuffer::getIndexBufferType() const
    {
        return m_type;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////


    /**
    * @brief UnorderedAccessBuffer class. Game side
    */
    class UnorderedAccessBuffer : public Buffer
    {
    public:

        /**
        * @brief getSize in bytes
        * @return Size in bytes
        */
        u32 getSize() const;

    public:

        /**
        * @brief UnorderedAccessBuffer constructor. Used to create UAV
        * @param BufferUsageFlags usage [required]
        * @param u32 size [required]
        * @param const std::string& name [optional]
        */
        explicit UnorderedAccessBuffer(Device* device, BufferUsageFlags usage, u32 size, const std::string& name = "") noexcept;

        /**
        * @brief UnorderedAccessBuffer destructor
        */
        ~UnorderedAccessBuffer();

    private:

        UnorderedAccessBuffer(const UnorderedAccessBuffer&) = delete;
        UnorderedAccessBuffer& operator=(const UnorderedAccessBuffer&) = delete;

        Device* const m_device;
        const u32     m_size;
    };

    inline u32 UnorderedAccessBuffer::getSize() const
    {
        return m_size;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief GeometryBufferDesc struct
    */
    struct GeometryBufferDesc
    {
        GeometryBufferDesc() noexcept
            : _indexBuffer(BufferHandle())
            , _indexOffset(0)
            , _indexType(IndexBufferType::IndexType_32)
            , _vertexBufferCount(0)
        {
        }

        GeometryBufferDesc(const VertexBuffer* vertex, u32 stride) noexcept
            : _indexBuffer(BufferHandle())
            , _indexOffset(0)
            , _indexType(IndexBufferType::IndexType_32)
            , _vertexBufferCount(1)
        {
            _vertexBuffers[0] = vertex->m_buffer;
            _vertexOffsets[0] = 0;
            _vertexStrides[0] = stride;
        }

        GeometryBufferDesc(const VertexBuffer* vertex, u32 stride, u32 offset) noexcept
            : _indexBuffer(BufferHandle())
            , _indexOffset(0)
            , _indexType(IndexBufferType::IndexType_32)
            , _vertexBufferCount(1)
        {
            _vertexBuffers[0] = vertex->m_buffer;
            _vertexOffsets[0] = offset;
            _vertexStrides[0] = stride;
        }

        GeometryBufferDesc(const IndexBuffer* index, u32 indexOffset, const VertexBuffer* vertex, u32 stride, u32 vertexOffset) noexcept
            : _indexBuffer(index->m_buffer)
            , _indexOffset(indexOffset)
            , _indexType(index->getIndexBufferType())
            , _vertexBufferCount(1)
        {
            _vertexBuffers[0] = vertex->m_buffer;
            _vertexOffsets[0] = vertexOffset;
            _vertexStrides[0] = stride;
        }

        ~GeometryBufferDesc() = default;

        BufferHandle                                        _indexBuffer;
        u64                                                 _indexOffset;
        IndexBufferType                                     _indexType;
        u32                                                 _vertexBufferCount = 0;
        std::array<BufferHandle, k_maxVertexInputBindings>  _vertexBuffers     = {};
        std::array<u64, k_maxVertexInputBindings>           _vertexOffsets     = {};
        std::array<u64, k_maxVertexInputBindings>           _vertexStrides     = {};
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer

    ///////////////////////////////////////////////////////////////////////////////////////////////////////

    template<>
    struct TypeOf<renderer::UnorderedAccessBuffer>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////////////


} //namespace v3d
