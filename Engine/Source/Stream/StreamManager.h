#pragma once

#include "MemoryStream.h"

namespace v3d
{
namespace utils
{
    class MemoryPool;
} //namespace utils

namespace stream
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * StreamManager class
    */
    class StreamManager
    {
    public:

        static MemoryStream* createMemoryStream(const void* data = nullptr, const u32 size = 0);

    private:

        static utils::MemoryPool* m_memoryPool;
    };



    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace stream
} //namespace v3d