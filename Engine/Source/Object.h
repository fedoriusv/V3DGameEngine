#pragma once

#include "Common.h"
#include "Utils/RefCounted.h"
#include "Utils/ResourceID.h"

#include "Utils/CRC32.h"
#include "crc32c/crc32c.h"
#include "ThirdParty/MurmurHash3.h"


namespace v3d
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Type;
    using TypePtr = Type const*;

    template<typename ObjectType>
    struct TypeOf
    {
        static TypePtr get()
        {
            ASSERT(false, "must not be called");
            return TypeOf<ObjectType>::get();
        }
    };

    template<typename ObjectType>
    TypePtr typeOf()
    {
        return TypeOf<ObjectType>::get();
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief ObjectHandle
    */
    struct ObjectHandle
    {
        ObjectHandle() noexcept
            : _type(nullptr)
            , _object(nullptr)
        {
        }

        template<typename ObjectType>
        ObjectHandle(ObjectType* object) noexcept
            : _type(typeOf<ObjectType>())
            , _object(object)
        {
        }

        ObjectHandle(TypePtr type) noexcept
            : _type(type)
            , _object(nullptr)
        {
        }

        bool isValid() const
        {
            return _object != nullptr && _type != nullptr;
        }

        template<typename ObjectType>
        bool isType() const
        {
            return _type == typeOf<ObjectType>();
        }

        TypePtr _type;
        void*   _object;
    };

    template<typename ObjectType>
    ObjectType* objectFromHandle(ObjectHandle handle)
    {
        ASSERT(handle.isType<ObjectType>(), "different types");
        return reinterpret_cast<ObjectType*>((handle)._object);
    }

    template<typename ObjectType>
    constexpr ObjectHandle makeObjectHandle(ObjectType* object = nullptr)
    {
        return ObjectHandle(object);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Object base game class. Game side
    */
    class V3D_API Object : public utils::RefCounted
    {
    public:

        Object() = default;
        virtual ~Object() = default;

        void* operator new(size_t size) noexcept
        {
            return V3D_MALLOC(size, memory::MemoryLabel::MemoryObject);
        }
        
        void operator delete(void* ptr) noexcept
        {
            V3D_FREE(ptr, memory::MemoryLabel::MemoryObject);
        }
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<typename TData>
    struct HasherCRC32
    {
        static u64 hash(const TData& data) noexcept
        {
            return static_cast<u64>(crc32c::Crc32c(reinterpret_cast<const c8*>(&data), sizeof(TData)));
        }
    };

    template<typename TData>
    struct HasherMurmur32
    {
        static u64 hash(const TData& data) noexcept
        {
            u32 hash = 0;
            MurmurHash3_x86_32(reinterpret_cast<const void*>(&data), sizeof(TData), 0, &hash);
            return static_cast<u64>(hash);
        }
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<class TDesc, typename Hasher = HasherMurmur32<TDesc>>
    struct DescInfo
    {
        DescInfo() noexcept = default;

        DescInfo(const TDesc& desc) noexcept
            : m_desc(desc)
            , m_hash(0)
        {
            m_hash = Hasher::hash(m_desc);
        }

        struct Hash
        {
            u64 operator()(const DescInfo<TDesc, Hasher>& desc) const
            {
                ASSERT(desc.m_hash, "must be calculated");
                return desc.m_hash;
            }
        };

        struct Compare
        {
            bool operator()(const DescInfo<TDesc, Hasher>& op1, const DescInfo<TDesc, Hasher>& op2) const
            {
                if constexpr (std::equality_comparable<TDesc>)
                {
                    return op1.m_desc == op2.m_desc;
                }
                else
                {
                    return memcmp(&op1.m_desc, &op2.m_desc, sizeof(TDesc)) == 0;
                }
            }
        };

        TDesc   m_desc;
        u64     m_hash;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace v3d
