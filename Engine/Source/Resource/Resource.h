#pragma once

#include "Common.h"
#include "Object.h"
#include "Utils/Observable.h"
#include "Utils/ResourceID.h"

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
    * @brief ResourceType enum
    * TODO: static RTTI
    */
    enum class ResourceType : u16
    {
        EmptyResource = 0,
        ModelResource,
        MeshResource,
        BitmapResource,
        MaterialResource,
        ShaderResource,

        ResourceType_Count
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief ResourceHeader meta info about Resource
    */
    struct ResourceHeader
    {
        static const u32 k_nameSize = 64;

        static bool validate(const ResourceHeader* header);
        static void fill(ResourceHeader* header, const std::string& name, u32 size, u32 offset, u32 flags = 0);

        ResourceHeader() noexcept = default;
        explicit ResourceHeader(ResourceType type) noexcept;
        explicit ResourceHeader(const ResourceHeader& other) noexcept;

        ~ResourceHeader() = default;

        ResourceType getResourceType() const;

        void setName(const std::string& name);

        u32 operator>>(stream::Stream* stream) const;
        u32 operator<<(const stream::Stream* stream);

    private:

        u16             _head;
        ResourceType    _type;
        u16             _version;
        u16             _flags;

    public:

        u32             _size;
        u32             _offset;

        u64             _timestamp;
        u64             _uID;
        u8              _name[k_nameSize];
    };

    inline ResourceType ResourceHeader::getResourceType() const
    {
        return _type;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Base Interface for Resource class
    */
    class Resource : public utils::Observable, public utils::ResourceID<Resource, u64>
    {
    public:

        Resource() noexcept = default;
        virtual ~Resource() = default;

        virtual bool load(const stream::Stream* stream, u32 offset = 0) = 0;
        virtual bool save(stream::Stream* stream, u32 offset = 0) const = 0;

    protected:

        Resource(const Resource&) = delete;
        Resource& operator=(const Resource&) = delete;

        bool m_loaded = false;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
