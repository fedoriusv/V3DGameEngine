#pragma once

#include "Common.h"
#include "MemoryStream.h"

namespace v3d
{
namespace stream
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * StreamManager class
    */
    class StreamManager
    {
    public:

        static [[nodiscard]] MemoryStream* createMemoryStream(const void* data = nullptr, const u32 size = 0);
        static [[nodiscard]] const MemoryStream* createMemoryStream(const std::string& string);

        static void destroyStream(const Stream* stream);
    };



    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace stream
} //namespace v3d
