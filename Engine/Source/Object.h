#pragma once

#include "Common.h"
#include "Utils/RefCounted.h"
#include "Utils/Resource.h"

#include "Utils/CRC32.h"
#include "crc32c/crc32c.h"


namespace v3d
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<typename ObjectPtrType = void*>
    struct ObjectHandle
    {
        explicit ObjectHandle(ObjectPtrType object = nullptr) noexcept
            : _object(object)
        {
            static_assert(std::is_pointer<ObjectPtrType>::value, "must be ptr");
        }

        bool isValid() const
        { 
            return _object != nullptr;
        }

        void reset()
        { 
            _object = nullptr;
        }

        bool operator==(const ObjectHandle<ObjectPtrType>& object) const
        { 
            return _object == object._object;
        }

        bool operator!=(const ObjectHandle<ObjectPtrType>& object) const
        { 
            return _object != object._object;
        }

        ObjectPtrType _object;
    };

#define OBJECT_FROM_HANDLE(handle, type) reinterpret_cast<type*>((handle)._object)

    template<typename ObjectHandle, typename ObjectPtrType = void*>
    constexpr ObjectPtrType objectFromHandle(ObjectHandle handle)
    {
        return reinterpret_cast<ObjectPtrType*>((handle)._object);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Object base game class. Game side
    */
    class Object : public utils::RefCounted
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
    struct HasherCRC
    {
        static u64 hash(const TData& data) noexcept
        {
            return static_cast<u64>(crc32c::Crc32c(reinterpret_cast<const c8*>(&data), sizeof(TData)));
        }
    };

    template<class TDesc, typename Hasher = HasherCRC<TDesc>>
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
            u64 operator()(const DescInfo<TDesc>& desc) const
            {
                ASSERT(desc.m_hash, "must be calculated");
                return desc.m_hash;
            }
        };

        struct Compare
        {
            bool operator()(const DescInfo<TDesc>& op1, const DescInfo<TDesc>& op2) const
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
