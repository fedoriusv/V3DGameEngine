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

        StreamBuffer() = default;
        virtual ~StreamBuffer() {};
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VertexStreamBuffer class. Game side
    */
    class VertexStreamBuffer : public StreamBuffer, public utils::Observer
    {
    public:

        VertexStreamBuffer() = delete;
        VertexStreamBuffer(const VertexStreamBuffer&) = delete;
        ~VertexStreamBuffer();

        bool update(u32 offset, u64 size, const u8* data);
        bool read(u32 offset, u64 size, u8* data); //TODO: add callback for async

    private:

        void handleNotify(const utils::Observable* ob) override;

        /**
        * @brief VertexStreamBuffer constructor. Used to create buffer data objects.
        * @param StreamBufferUsageFlags usage [required]
        * @param u64 size [required] in bytes
        * @param  const u8* data [required]
        * @param const std::string& name [optional]
        */
        explicit VertexStreamBuffer(CommandList& cmdList, StreamBufferUsageFlags usage, u64 size, const u8* data, [[maybe_unused]] const std::string& name = "") noexcept;

        friend CommandList;
        CommandList& m_cmdList;

        u64 m_size;
        void* m_data;
                
        StreamBufferUsageFlags m_usage;
        Buffer* m_buffer;

        friend StreamBufferDescription;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief IndexStreamBuffer class. Game side
    */
    class IndexStreamBuffer : public StreamBuffer, public utils::Observer
    {
    public:

        IndexStreamBuffer() = delete;
        IndexStreamBuffer(const IndexStreamBuffer&) = delete;
        ~IndexStreamBuffer();

        u32 getIndexCount() const;
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
        explicit IndexStreamBuffer(CommandList& cmdList, StreamBufferUsageFlags usage, StreamIndexBufferType type, u32 count, const u8* data, [[maybe_unused]] const std::string& name = "") noexcept;

        void handleNotify(const utils::Observable* ob) override;

        friend CommandList;
        CommandList& m_cmdList;

        StreamIndexBufferType m_type;
        u32 m_count;
        void* m_data;

        StreamBufferUsageFlags m_usage;
        Buffer* m_buffer;

        friend StreamBufferDescription;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
