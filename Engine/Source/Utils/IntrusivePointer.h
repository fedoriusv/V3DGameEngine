#pragma once

namespace v3d
{
namespace utils
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template <class T>
    class IntrusivePointer
    {
    private:

        typedef IntrusivePointer this_type;

    public:

        IntrusivePointer();
        IntrusivePointer(T* p, bool add_ref = true);
        IntrusivePointer(IntrusivePointer const& rhs);
        ~IntrusivePointer();

        IntrusivePointer&   operator=(IntrusivePointer const& rhs);
        IntrusivePointer&   operator=(T* rhs);
        T&                  operator*() const;
        T*                  operator->() const;
        T*                  get() const;

        void                reset();
        void                reset(T* rhs);
        void                swap(IntrusivePointer& rhs);

        operator            bool() const;

    private:

        T* m_px;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template <class T>
    IntrusivePointer<T>::IntrusivePointer()
        : m_px(nullptr)
    {
    }

    template <class T>
    IntrusivePointer<T>::IntrusivePointer(T* p, bool add_ref)
        : m_px(p)
    {
        if (m_px != nullptr && add_ref)
        {
            intrusivePtrAddRef(m_px);
        }
    }

    template <class T>
    IntrusivePointer<T>::IntrusivePointer(IntrusivePointer const& rhs)
        : m_px(rhs.m_px)
    {
        if (m_px != nullptr)
        {
            intrusivePtrAddRef(m_px);
        }
    }

    template <class T>
    IntrusivePointer<T>::~IntrusivePointer()
    {
        if (m_px != nullptr)
        {
            intrusivePtrRelease(m_px);
        }
    }

    template <class T>
    IntrusivePointer<T>& IntrusivePointer<T>::operator=(IntrusivePointer const& rhs)
    {
        this_type(rhs).swap(*this);
        return *this;
    }

    template <class T>
    IntrusivePointer<T>& IntrusivePointer<T>::operator=(T* rhs)
    {
        this_type(rhs).swap(*this);
        return *this;
    }

    template <class T>
    void IntrusivePointer<T>::reset()
    {
        this_type().swap(*this);
    }

    template <class T>
    void IntrusivePointer<T>::reset(T* rhs)
    {
        this_type(rhs).swap(*this);
    }

    template <class T>
    T* IntrusivePointer<T>::get() const
    {
        return m_px;
    }

    template <class T>
    T& IntrusivePointer<T>::operator*() const
    {
        return *m_px;
    }

    template <class T>
    T* IntrusivePointer<T>::operator->() const
    {
        return m_px;
    }

    template <class T>
    IntrusivePointer<T>::operator bool() const
    {
        return m_px != nullptr;
    }

    template <class T>
    void IntrusivePointer<T>::swap(IntrusivePointer<T>& rhs)
    {
        T* tmp = m_px;
        m_px = rhs.m_px;
        rhs.m_px = tmp;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<class T, class U> 
    inline bool operator==(IntrusivePointer<T> const& a, IntrusivePointer<U> const& b)
    {
        return a.get() == b.get();
    }

    template<class T, class U>
    inline bool operator!=(IntrusivePointer<T> const& a, IntrusivePointer<U> const& b)
    {
        return a.get() != b.get();
    }

    template<class T, class U>
    inline bool operator==(IntrusivePointer<T> const& a, U* b)
    {
        return a.get() == b;
    }

    template<class T, class U>
    inline bool operator!=(IntrusivePointer<T> const& a, U* b)
    {
        return a.get() != b;
    }

    template<class T, class U>
    inline bool operator==(T* a, IntrusivePointer<U> const& b)
    {
        return a == b.get();
    }

    template<class T, class U>
    inline bool operator!=(T* a, IntrusivePointer<U> const& b)
    {
        return a != b.get();
    }

    template<class T>
    inline bool operator<(IntrusivePointer<T> const& a, IntrusivePointer<T> const& b)
    {
        return std::less<T*>()(a.get(), b.get());
    }

    template<class T>
    void swap(IntrusivePointer<T>& lhs, IntrusivePointer<T>& rhs)
    {
        lhs.swap(rhs);
    }

    template<class T>
    T * get_pointer(IntrusivePointer<T> const& p)
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
