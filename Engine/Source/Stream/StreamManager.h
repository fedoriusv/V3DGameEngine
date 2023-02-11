#pragma once

#include "Common.h"
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
        static const MemoryStream* createMemoryStream(const std::string& string);

        static void clearPools();

    private:

        static utils::MemoryPool* s_memoryPool;
    };



    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace stream
} //namespace v3d
