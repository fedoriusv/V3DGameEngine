namespace v3d
{
namespace math
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template <class T>
    TMinMaxAABB<T>::TMinMaxAABB() noexcept
        : _min(k_infinity)
        , _max(-k_infinity)
    {
    }
    
    template <class T>
    TMinMaxAABB<T>::TMinMaxAABB(const TMinMaxAABB<T>& other) noexcept
        : _min(other._min)
        , _max(other._max)
    {
    }

    template <class T>
    TMinMaxAABB<T>::TMinMaxAABB(const TVector3D<T>& min, const TVector3D<T>& max) noexcept
        : _min(min)
        , _max(max)
    {
    }

    template <class T>
    inline TMinMaxAABB<T>& TMinMaxAABB<T>::operator=(const TMinMaxAABB<T>& other) const
    {
        if (this == &other)
        {
            return *this;
        }

        _min = other._min;
        _max = other._max;

        return *this;
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
        return _min == other._min && _max == other._max;
    }
    
    template <class T>
    inline bool TMinMaxAABB<T>::operator!=(const TMinMaxAABB<T>& other) const
    {
        return !(*this == other);
    }

    template <class T>
    inline const TVector3D<T>& TMinMaxAABB<T>::getMin() const
    {
        return _min;
    }

    template <class T>
    inline const TVector3D<T>& TMinMaxAABB<T>::getMax() const
    {
        return _max;
    }

    template <class T>
    inline TVector3D<T> TMinMaxAABB<T>::getCenter() const
    {
        return (_max + _min) / 2.0f;
    }

    template <class T>
    inline TVector3D<T> TMinMaxAABB<T>::getExtent() const
    {
        return 0.5f * (_max - _min);
    }

    template <class T>
    inline TVector3D<T> TMinMaxAABB<T>::getSize() const
    {
        return _max - _min;
    }

    template <class T>
    void TMinMaxAABB<T>::expand(T value)
    {
        TVector3D<T> offset(value);
        _min -= offset;
        _max += offset;
    }

    template <class T>
    void TMinMaxAABB<T>::expand(const TVector3D<T>& value)
    {
        _min -= value;
        _max += value;
    }

    template <class T>
    bool TMinMaxAABB<T>::isValid() const
    {
        return _min != k_infinity && _max != -k_infinity;
    }

    template <class T>
    bool TMinMaxAABB<T>::isInside(const TVector3D<T>& point) const
    {
        if (point._x < _min._x)
        {
            return false;
        }

        if (point._x > _max._x)
        {
            return false;
        }

        if (point._y < _min._y)
        {
            return false;
        }

        if (point._y > _max._y)
        {
            return false;
        }

        if (point._z < _min._z)
        {
            return false;
        }

        if (point._z > _max._z)
        {
            return false;
        }

        return true;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace math
} //namespace v3d