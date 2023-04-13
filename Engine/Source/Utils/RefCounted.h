#pragma once

#include "Common.h"

namespace v3d
{
namespace utils
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief RefCounted class.
    * @see IntrusivePointer
    */
    class RefCounted
    {
    public:

        explicit RefCounted(s32 refCount = 0) noexcept;
        virtual ~RefCounted() = default;

        s32 getCount() const;

    private:

        void grab() const;
        void drop() const;

        friend void intrusivePtrAddRef(const RefCounted* obj);
        friend void intrusivePtrRelease(const RefCounted* obj);

        mutable s32 m_refCount;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    inline RefCounted::RefCounted(s32 refCount) noexcept
        : m_refCount(refCount)
    {
    }

    inline s32 RefCounted::getCount() const
    {
        return m_refCount;
    }

    inline void RefCounted::grab() const
    {
        ++m_refCount;
    }

    inline void RefCounted::drop() const
    {
        --m_refCount;
        if (m_refCount == 0)
        {
            //TODO memory handle
            delete this;
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    inline void intrusivePtrAddRef(const RefCounted* obj)
    {
        obj->grab();
    }

    inline void intrusivePtrRelease(const RefCounted* obj)
    {
        obj->drop();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace utils
} //namespace v3d
