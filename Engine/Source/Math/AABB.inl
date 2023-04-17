namespace v3d
{
namespace math
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template <class T>
    TMinMaxAABB<T>::TMinMaxAABB() noexcept
        : m_min(k_infinity)
        , m_max(-k_infinity)
    {
    }
    
    template <class T>
    TMinMaxAABB<T>::TMinMaxAABB(const TMinMaxAABB<T>& other) noexcept
        : m_min(other.m_min)
        , m_max(other.m_max)
    {
    }

    template <class T>
    TMinMaxAABB<T>::TMinMaxAABB(const TVector3D<T>& min, const TVector3D<T>& max) noexcept
        : m_min(min)
        , m_max(max)
    {
    }

    template <class T>
    inline TMinMaxAABB<T>& TMinMaxAABB<T>::operator=(const TMinMaxAABB<T>& other) const
    {
        if (this == &other)
        {
            return *this;
        }

        m_min = other.m_min;
        m_max = other.m_max;
    }

    template <class T>
    T* TMinMaxAABB<T>::getPtr()
    {
        return (T*)this;
    }
    
    template <class T>
    const T* TMinMaxAABB<T>::getPtr() const
    {
        return (T*)this;
    }

    template <class T>
    inline bool TMinMaxAABB<T>::operator==(const TMinMaxAABB<T>& other) const
    {
        return m_min == other.m_min && m_max == other.m_max;
    }
    
    template <class T>
    inline bool TMinMaxAABB<T>::operator!=(const TMinMaxAABB<T>& other) const
    {
        return !(*this == other);
    }

    template <class T>
    inline const TVector3D<T>& TMinMaxAABB<T>::getMin() const
    {
        return m_min;
    }

    template <class T>
    inline const TVector3D<T>& TMinMaxAABB<T>::getMax() const
    {
        return m_max;
    }

    template <class T>
    inline TVector3D<T> TMinMaxAABB<T>::getCenter() const
    {
        return (m_max + m_min) / 2.0f;
    }

    template <class T>
    inline TVector3D<T> TMinMaxAABB<T>::getExtent() const
    {
        return 0.5f * (m_max - m_min);
    }

    template <class T>
    inline TVector3D<T> TMinMaxAABB<T>::getSize() const
    {
        return m_max - m_min;
    }

    template <class T>
    void TMinMaxAABB<T>::expand(T value)
    {
        TVector3D<T> offset(value);
        m_min -= offset;
        m_max += offset;
    }

    template <class T>
    void TMinMaxAABB<T>::expand(const TVector3D<T>& value)
    {
        m_min -= value;
        m_max += value;
    }

    template <class T>
    bool TMinMaxAABB<T>::isValid() const
    {
        return m_min != k_infinity && m_max != -k_infinity;
    }

    template <class T>
    bool TMinMaxAABB<T>::isInside(const TVector3D<T>& point) const
    {
        if (point.m_x < m_min.m_x)
        {
            return false;
        }

        if (point.m_x > m_max.m_x)
        {
            return false;
        }


        if (point.m_y < m_min.m_y)
        {
            return false;
        }

        if (point.m_y > m_max.m_y)
        {
            return false;
        }


        if (point.m_z < m_min.m_z)
        {
            return false;
        }

        if (point.m_z > m_max.m_z)
        {
            return false;
        }

        return true;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace math
} //namespace v3d