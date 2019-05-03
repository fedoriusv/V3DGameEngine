#pragma once

#include "Stream/Stream.h"

namespace v3d
{
namespace resource
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * ResourceHeader meta info about Resource
    */
    struct ResourceHeader
    {
        ResourceHeader()
            : _size(0)
            , _version(0)
            , _flags(0)
        {
        }

        virtual ~ResourceHeader() {};

        u32 _size;
        u32 _version;
        u32 _flags;
#if DEBUG
        std::string _debugName;
#endif
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Base Interface for Resource class
    */
    class Resource
    {
    public:

        Resource(const Resource&) = delete;
        Resource& operator=(const Resource&) = delete;

        Resource() noexcept
            : m_header(nullptr)
            , m_stream(nullptr)
            , m_loaded(false)
        {
        }

        explicit Resource(const ResourceHeader* header) noexcept
            : m_header(header)
            , m_stream(nullptr)
            , m_loaded(false)
        {
        }

        virtual ~Resource() 
        {
            if (m_header)
            {
                delete m_header;
            }

            if (m_stream)
            {
                ASSERT(!m_stream->isMapped(), "mapped");
                delete m_stream;
                m_stream = nullptr;
            }
        };

        virtual void init(stream::Stream* stream) = 0;
        virtual bool load() = 0;

    protected:

        const ResourceHeader*     m_header;
        stream::Stream*           m_stream;
        
        bool                      m_loaded;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
