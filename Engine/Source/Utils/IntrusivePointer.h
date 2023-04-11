#pragma once

#include "Common.h"

namespace v3d
{
namespace utils
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief IntrusivePointer interface. Smart pointer
    */
    template <class TType>
    class IntrusivePointer
    {
    private:

        typedef IntrusivePointer this_type;

    public:

        IntrusivePointer() noexcept;
        explicit IntrusivePointer(TType* p, bool addRef = true) noexcept;
        IntrusivePointer(IntrusivePointer const& rhs) noexcept;
        ~IntrusivePointer();

        IntrusivePointer&   operator=(IntrusivePointer const& rhs);
        IntrusivePointer&   operator=(TType* rhs);
        TType&               operator*() const;
        TType*               operator->() const;
        TType*               get() const;

        void                reset();
        void                reset(TType* rhs);
        void                swap(IntrusivePointer& rhs);

        operator            bool() const;

    private:

        TType* m_px;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template <class TType>
    IntrusivePointer<TType>::IntrusivePointer() noexcept
        : m_px(nullptr)
    {
    }

    template <class TType>
    IntrusivePointer<TType>::IntrusivePointer(TType* p, bool addRef) noexcept
        : m_px(p)
    {
        if (m_px != nullptr && addRef)
        {
            intrusivePtrAddRef(m_px);
        }
    }

    template <class TType>
    IntrusivePointer<TType>::IntrusivePointer(IntrusivePointer const& rhs) noexcept
        : m_px(rhs.m_px)
    {
        if (m_px != nullptr)
        {
            intrusivePtrAddRef(m_px);
        }
    }

    template <class TType>
    IntrusivePointer<TType>::~IntrusivePointer()
    {
        if (m_px != nullptr)
        {
            intrusivePtrRelease(m_px);
        }
    }

    template <class TType>
    IntrusivePointer<TType>& IntrusivePointer<TType>::operator=(IntrusivePointer const& rhs)
    {
        this_type(rhs).swap(*this);
        return *this;
    }

    template <class TType>
    IntrusivePointer<TType>& IntrusivePointer<TType>::operator=(TType* rhs)
    {
        this_type(rhs).swap(*this);
        return *this;
    }

    template <class TType>
    void IntrusivePointer<TType>::reset()
    {
        this_type().swap(*this);
    }

    template <class TType>
    void IntrusivePointer<TType>::reset(TType* rhs)
    {
        this_type(rhs).swap(*this);
    }

    template <class TType>
    TType* IntrusivePointer<TType>::get() const
    {
        return m_px;
    }

    template <class TType>
    TType& IntrusivePointer<TType>::operator*() const
    {
        return *m_px;
    }

    template <class TType>
    TType* IntrusivePointer<TType>::operator->() const
    {
        return m_px;
    }

    template <class TType>
    IntrusivePointer<TType>::operator bool() const
    {
        return m_px != nullptr;
    }

    template <class TType>
    void IntrusivePointer<TType>::swap(IntrusivePointer<TType>& rhs)
    {
        TType* tmp = m_px;
        m_px = rhs.m_px;
        rhs.m_px = tmp;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<class T, class U> 
    bool operator==(IntrusivePointer<T> const& a, IntrusivePointer<U> const& b)
    {
        return a.get() == b.get();
    }

    template<class T, class U>
    bool operator!=(IntrusivePointer<T> const& a, IntrusivePointer<U> const& b)
    {
        return a.get() != b.get();
    }

    template<class T, class U>
    bool operator==(IntrusivePointer<T> const& a, U* b)
    {
        return a.get() == b;
    }

    template<class T, class U>
    bool operator!=(IntrusivePointer<T> const& a, U* b)
    {
        return a.get() != b;
    }

    template<class T, class U>
    bool operator==(T* a, IntrusivePointer<U> const& b)
    {
        return a == b.get();
    }

    template<class T, class U>
    bool operator!=(T* a, IntrusivePointer<U> const& b)
    {
        return a != b.get();
    }

    template<class T>
    bool operator<(IntrusivePointer<T> const& a, IntrusivePointer<T> const& b)
    {
        return std::less<T*>()(a.get(), b.get());
    }

    template<class T>
    void swap(IntrusivePointer<T>& lhs, IntrusivePointer<T>& rhs)
    {
        lhs.swap(rhs);
    }

    template<class T>
    T* get_pointer(IntrusivePointer<T> const& p)
    {
        return p.get();
    }

    template<class T, class U>
    IntrusivePointer<T> static_pointer_cast(IntrusivePointer<U> const& p)
    {
        return static_cast<T*>(p.get());
    }

    template<class T, class U>
    IntrusivePointer<T> const_pointer_cast(IntrusivePointer<U> const& p)
    {
        return const_cast<T*>(p.get());
    }

    template<class T, class U>
    IntrusivePointer<T> dynamic_pointer_cast(IntrusivePointer<U> const& p)
    {
        return dynamic_cast<T*>(p.get());
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace utils
} //namespace v3d
