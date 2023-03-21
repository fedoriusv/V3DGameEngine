#pragma once

#include "Common.h"
#include "Utils/Observable.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Context;

    /**
    * @brief Buffer base class. Render side
    */
    class Buffer : public utils::Observable
    {
    public:

        /**
        * @brief BufferType enum
        */
        enum class BufferType : u32
        {
            BufferType_VertexBuffer,
            BufferType_IndexBuffer,
            BufferType_ConstantBuffer,
            BufferType_StagingBuffer,
            BufferType_Readback
        };

        Buffer() = default;
        virtual ~Buffer() = default;

        virtual bool create() = 0;
        virtual void destroy() = 0;

        virtual bool write(Context* context, u32 offset, u64 size, const void* data) = 0;
        virtual bool read(Context* context, u32 offset, u64 size, const std::function<void(u32, void*)>& readback) = 0;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
