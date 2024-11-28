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

        explicit RefCounted(s64 refCount = 0) noexcept;
        virtual ~RefCounted() = default;

        s64 getRefs() const;

    private:

        void grab() const;
        void drop() const;

        friend void intrusivePtrAddRef(const RefCounted* obj);
        friend void intrusivePtrRelease(const RefCounted* obj);

        mutable std::atomic<s64> m_refCount;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    inline RefCounted::RefCounted(s64 refCount) noexcept
        : m_refCount(refCount)
    {
    }

    inline s64 RefCounted::getRefs() const
    {
        return m_refCount;
    }

    inline void RefCounted::grab() const
    {
        m_refCount.fetch_add(1, std::memory_order_relaxed);
    }

    inline void RefCounted::drop() const
    {
        m_refCount.fetch_sub(1, std::memory_order_relaxed);
        if (m_refCount.load(std::memory_order_relaxed) == 0)
        {
            //handle by Object new/delete. @see Object class
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
