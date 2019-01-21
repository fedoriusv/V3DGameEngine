#pragma once

#include "Common.h"

namespace v3d
{
namespace renderer
{
    class VertexStreamBuffer;

    ///////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * StreamBufferUsage enum. usageFlag inside StreamBuffer
    */
    enum StreamBufferUsage
    {
        StreamBuffer_Upload = 0x01,
        StreamBuffer_Readback = 0x02,
        StreamBuffer_Direct = 0x04,
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * StreamBufferData struct
    */
    struct StreamBufferData
    {
        StreamBufferData();
        StreamBufferData(u32 size, void* data);

        void lock();
        void unlock();

        u32                 _size;
        void*               _data;

    private:
        std::atomic<bool>   _lock;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * StreamBufferDescription struct
    */
    struct StreamBufferDescription
    {
        StreamBufferDescription() = delete;
        StreamBufferDescription(StreamBufferDescription&) = delete;

        StreamBufferDescription(VertexStreamBuffer* vertex, u32 streamId);
        ~StreamBufferDescription();

        u16 _usageFlag;
        StreamBufferData* _stream;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
