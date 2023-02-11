#pragma once

#include "Common.h"
#include "Object.h"
#include "Utils/Observable.h"

#include "Formats.h"
#include "BufferProperties.h"

#include "CommandList.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Buffer;
    struct StreamBufferDescription;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief StreamBuffer base class. Game side
    */
    class StreamBuffer : public Object
    {
    protected:

        /**
        * @brief StreamBuffer destructor
        */
        virtual ~StreamBuffer();

        /**
        * @brief StreamBuffer constructor
        * @param CommandList& cmdList [required]
        * @param  StreamBufferUsageFlags usage [required]
        * @param const std::string& name [optional]
        */
        StreamBuffer(CommandList& cmdList, StreamBufferUsageFlags usage, [[maybe_unused]] const std::string& name = "") noexcept;

        CommandList& m_cmdList;

        StreamBufferUsageFlags m_usage;
        [[maybe_unused]] const std::string m_name;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VertexStreamBuffer class. Game side
    */
    class VertexStreamBuffer : public StreamBuffer, public utils::Observer
    {
    public:

        /**
        * @brief VertexStreamBuffer destructor
        */
        ~VertexStreamBuffer();

        /**
        * @brief update
        * @param u32 offset [required]
        * @param u64 size [required] in bytes
        * @param const u8* data [required]
        */
        bool update(u32 offset, u64 size, const u8* data);

        /**
        * @brief read
        * @param u32 offset [required]
        * @param u64 size [required] in bytes
        * @param const u8* data [required]
        */
        bool read(u32 offset, u64 size, u8* data); //TODO: add callback for async

    private:

        /**
        * @brief VertexStreamBuffer constructor. Used to create buffer data objects.
        * @param StreamBufferUsageFlags usage [required]
        * @param u64 size [required] in bytes
        * @param  const u8* data [required]
        * @param const std::string& name [optional]
        */
        VertexStreamBuffer(CommandList& cmdList, StreamBufferUsageFlags usage, u64 size, const u8* data, [[maybe_unused]] const std::string& name = "") noexcept;

        VertexStreamBuffer() = delete;
        VertexStreamBuffer(const VertexStreamBuffer&) = delete;

        void handleNotify(const utils::Observable* object, void* msg) override;

        u64 m_size;
        void* m_data;

        Buffer* m_buffer;

        friend CommandList;
        friend StreamBufferDescription;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief IndexStreamBuffer class. Game side
    */
    class IndexStreamBuffer : public StreamBuffer, public utils::Observer
    {
    public:

        /**
        * @brief IndexStreamBuffer destructor
        */
        ~IndexStreamBuffer();

        /**
        * @brief getIndexCount gets count of indexes.
        * @return Count of indexes
        */
        u32 getIndexCount() const;

        /**
        * @brief getIndexBufferType gets type of indexes.
        * @return type of indexes StreamIndexBufferType
        */
        StreamIndexBufferType getIndexBufferType() const;

    private:

        /**
        * @brief IndexStreamBuffer constructor. Used to create buffer index objects.
        * @param StreamBufferUsageFlags usage [required]
        * @param StreamIndexBufferType type [required]
        * @param u32 count [required]
        * @param  const u8* data [required]
        * @param const std::string& name [optional]
        */
        IndexStreamBuffer(CommandList& cmdList, StreamBufferUsageFlags usage, StreamIndexBufferType type, u32 count, const u8* data, [[maybe_unused]] const std::string& name = "") noexcept;

        IndexStreamBuffer() = delete;
        IndexStreamBuffer(const IndexStreamBuffer&) = delete;

        void handleNotify(const utils::Observable* object, void* msg) override;

        StreamIndexBufferType m_type;
        u32 m_count;
        void* m_data;

        Buffer* m_buffer;

        friend CommandList;
        friend StreamBufferDescription;
    };

    inline u32 IndexStreamBuffer::getIndexCount() const
    {
        return m_count;
    }

    inline StreamIndexBufferType IndexStreamBuffer::getIndexBufferType() const
    {
        return m_type;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
