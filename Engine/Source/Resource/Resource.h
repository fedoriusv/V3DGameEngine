#pragma once

#include "Stream/Stream.h"
#include "Utils/NonCopyable.h"

namespace v3d
{
namespace resource
{
    class ResourceCreator;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * ResourceHeader meta info about Resource
    */
    struct ResourceHeader
    {
        u32 _size;
        u32 _version;
        u32 _flags;
    };

    /**
    * Base Interface for Resource class
    */
    class Resource : public utils::NonCopyable
    {
    public:

        Resource() {};
        virtual ~Resource() {};

    private:

        Resource(const ResourceHeader* header, stream::Stream* stream)
            : m_header(header)
            , m_stream(stream)
        {
        }

        friend ResourceCreator;

        const ResourceHeader*     m_header;
        stream::Stream*           m_stream;
    };

    class ResourceCreator
    {
    public:

        template<class TResource>
        static Resource* create(const ResourceHeader* header, stream::Stream* stream)
        {
            return new TResource(nullptr, nullptr);
        }
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
