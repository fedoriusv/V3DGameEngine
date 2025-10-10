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
    */
    enum class ResourceType : u8
    {
        Empty = 0,
        Technique,
        Shader,
        Texture,
        Bitmap,
        Model,
        Mesh,
        Material,
        Billboard,
        Skybox,
        Light,
        Camera,

        Count
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief ResourceHeader meta info about Resource
    */
    struct V3D_API ResourceHeader
    {
        static const u32 k_nameSize = 64;

        static bool validate(const ResourceHeader* header);
        static void fill(ResourceHeader* header, const std::string& name, u32 size, u32 offset, u32 flags = 0);

        ResourceHeader() noexcept = default;
        explicit ResourceHeader(ResourceType type, u8 subType = 0) noexcept;
        explicit ResourceHeader(const ResourceHeader& other) noexcept;

        ~ResourceHeader() = default;

        ResourceType getResourceType() const;

        template<EnumType T>
        T getResourceSubType() const;

        void setName(const std::string& name);
        const std::string_view getName() const;

        u32 operator>>(stream::Stream* stream) const;
        u32 operator<<(const stream::Stream* stream);

    private:

        u16             _head;
        u16             _type;
        u16             _version;
        u16             _flags;

    public:

        u32             _size;
        u32             _offset;
        u64             _timestamp;
        u64             _uID;
        c8              _name[k_nameSize];
    };

    inline const std::string_view ResourceHeader::getName() const
    {
        return std::string_view(&_name[0]);
    }

    inline ResourceType ResourceHeader::getResourceType() const
    {
        return ResourceType(_type >> 8);
    }

    template<EnumType T>
    inline T ResourceHeader::getResourceSubType() const
    {
        return static_cast<T>(_type & 0xFF);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    struct ResourceReport
    {
        enum class Event
        {
            Destroy
        };

        Event _event;
    };

    /**
    * @brief Base Interface for Resource class
    */
    class V3D_API Resource : public utils::ResourceID<Resource, u64>, public utils::Reporter<ResourceReport>
    {
    public:

        virtual ~Resource() = default;

        virtual bool load(const stream::Stream* stream, u32 offset = 0) = 0;
        virtual bool save(stream::Stream* stream, u32 offset = 0) const = 0;

    protected:

        Resource() noexcept = default;
        Resource(const Resource&) = delete;
        Resource& operator=(const Resource&) = delete;

        bool m_loaded = false;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
