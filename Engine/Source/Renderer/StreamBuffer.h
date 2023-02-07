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
    public:

        virtual ~StreamBuffer();

    protected:

        StreamBuffer(CommandList& cmdList, StreamBufferUsageFlags usage, [[maybe_unused]] const std::string& name = "");

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

        ~VertexStreamBuffer();

        bool update(u32 offset, u64 size, const u8* data);
        bool read(u32 offset, u64 size, u8* data); //TODO: add callback for async

    private:

        VertexStreamBuffer() = delete;
        VertexStreamBuffer(const VertexStreamBuffer&) = delete;

        void handleNotify(const utils::Observable* ob) override;

        /**
        * @brief VertexStreamBuffer constructor. Used to create buffer data objects.
        * @param StreamBufferUsageFlags usage [required]
        * @param u64 size [required] in bytes
        * @param  const u8* data [required]
        * @param const std::string& name [optional]
        */
        VertexStreamBuffer(CommandList& cmdList, StreamBufferUsageFlags usage, u64 size, const u8* data, [[maybe_unused]] const std::string& name = "") noexcept;

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

        IndexStreamBuffer() = delete;
        IndexStreamBuffer(const IndexStreamBuffer&) = delete;

        /**
        * @brief IndexStreamBuffer constructor. Used to create buffer index objects.
        * @param StreamBufferUsageFlags usage [required]
        * @param StreamIndexBufferType type [required]
        * @param u32 count [required]
        * @param  const u8* data [required]
        * @param const std::string& name [optional]
        */
        IndexStreamBuffer(CommandList& cmdList, StreamBufferUsageFlags usage, StreamIndexBufferType type, u32 count, const u8* data, [[maybe_unused]] const std::string& name = "") noexcept;

        void handleNotify(const utils::Observable* ob) override;

        StreamIndexBufferType m_type;
        u32 m_count;
        void* m_data;

        Buffer* m_buffer;

        friend CommandList;
        friend StreamBufferDescription;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
