#pragma once

#include "Common.h"
#include "Utils/Observable.h"

namespace v3d
{
namespace stream
{
    class Stream;
} //namespace stream
namespace resource
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief ResourceHeader meta info about Resource
    */
    struct ResourceHeader
    {
        ResourceHeader() noexcept;
        virtual ~ResourceHeader() = default;

        virtual u32 operator>>(stream::Stream* stream);
        virtual u32 operator<<(const stream::Stream* stream);

        u32 _size;
        u32 _offset;
        u32 _version;
        u32 _extraFlags;
#if DEBUG
        std::string _name;
#endif
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Base Interface for Resource class
    */
    class Resource : public utils::Observable
    {
    public:

        Resource() noexcept;
        explicit Resource(const ResourceHeader* header) noexcept;
        virtual ~Resource();

        virtual void init(stream::Stream* stream) = 0;
        virtual bool load() = 0;

    protected:

        Resource(const Resource&) = delete;
        Resource& operator=(const Resource&) = delete;

        const ResourceHeader* m_header;
        stream::Stream* m_stream;
        
        bool m_loaded;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
