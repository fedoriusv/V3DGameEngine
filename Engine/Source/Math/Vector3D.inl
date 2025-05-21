namespace v3d
{
namespace math
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<class T>
    TVector3D<T>::TVector3D() noexcept
        : _x(0)
        , _y(0)
        , _z(0)
    {
    }

    template<class T>
    TVector3D<T>::TVector3D(T n) noexcept
        : _x(n)
        , _y(n)
        , _z(n)
    {
    }

    template<class T>
    TVector3D<T>::TVector3D(T x, T y, T z) noexcept
        : _x(x)
        , _y(y)
        , _z(z)
    {
    }

    template<class T>
    TVector3D<T>::TVector3D(const TVector3D<T>& other) noexcept
        : _x(other._x)
        , _y(other._y)
        , _z(other._z)
    {
    }

    template<class T>
    TVector3D<T>::TVector3D(const TVector2D<T>& other, T z) noexcept
        : _x(other._x)
        , _y(other._y)
        , _z(z)
    {
    }

    template<class T>
    inline TVector3D<T> TVector3D<T>::operator-() const
    {
        return TVector3D<T>(-_x, -_y, -_z);
    }

    template<class T>
    inline TVector3D<T>& TVector3D<T>::operator=(const TVector3D<T>& other)
    {
        if (this == &other)
        {
            return *this;
        }

        _x = other._x;
        _y = other._y;
        _z = other._z;
        return *this;
    }

    template<class T>
    inline TVector3D<T> TVector3D<T>::operator+(const TVector3D<T>& other) const
    {
        return TVector3D<T>(_x + other._x, _y + other._y, _z + other._z);
    }

    template<class T>
    inline TVector3D<T>& TVector3D<T>::operator+=(const TVector3D<T>& other)
    {
        _x += other._x;
        _y += other._y;
        _z += other._z;

        return *this;
    }

    template<class T>
    inline TVector3D<T> TVector3D<T>::operator+(T scalar) const
    {
        return TVector3D<T>(_x + scalar, _y + scalar, _z + scalar);
    }

    template<class T>
    inline TVector3D<T>& TVector3D<T>::operator+=(T scalar)
    {
        _x += scalar;
        _y += scalar;
        _z += scalar;

        return *this;
    }

    template<class T>
    inline TVector3D<T> TVector3D<T>::operator-(const TVector3D<T>& other) const
    {
        return TVector3D<T>(_x - other._x, _y - other._y, _z - other._z);
    }

    template<class T>
    inline TVector3D<T>& TVector3D<T>::operator-=(const TVector3D<T>& other)
    {
        _x -= other._x;
        _y -= other._y;
        _z -= other._z;

        return *this;
    }

    template<class T>
    inline TVector3D<T> TVector3D<T>::operator-(T scalar) const
    {
        return TVector3D<T>(_x - scalar, _y - scalar, _z - scalar);
    }

    template<class T>
    inline TVector3D<T>& TVector3D<T>::operator-=(T scalar)
    {
        _x -= scalar;
        _y -= scalar;
        _z -= scalar;

        return *this;
    }

    template<class T>
    inline TVector3D<T> TVector3D<T>::operator*(const TVector3D<T>& other) const
    {
        return TVector3D<T>(_x * other._x, _y * other._y, _z * other._z);
    }

    template<class T>
    inline TVector3D<T>& TVector3D<T>::operator*=(const TVector3D<T>& other)
    {
        _x *= other._x;
        _y *= other._y;
        _z *= other._z;

        return *this;
    }

    template<class T>
    inline TVector3D<T> TVector3D<T>::operator*(T scalar) const
    {
        return TVector3D<T>(_x * scalar, _y * scalar, _z * scalar);
    }

    template<class T>
    inline TVector3D<T>& TVector3D<T>::operator*=(T scalar)
    {
        _x *= scalar;
        _y *= scalar;
        _z *= scalar;

        return *this;
    }

    template<class T>
    inline TVector3D<T> TVector3D<T>::operator/(const TVector3D<T>& other) const
    {
        return TVector3D<T>(_x / other._x, _y / other._y, _z / other._z);
    }

    template<class T>
    inline TVector3D<T>& TVector3D<T>::operator/=(const TVector3D<T>& other)
    {
        _x /= other._x;
        _y /= other._y;
        _z /= other._z;

        return *this;
    }

    template<class T>
    inline TVector3D<T> TVector3D<T>::operator/(T scalar) const
    {
        T i = (T)1.0 / scalar;
        return TVector3D<T>(_x * i, _y * i, _z * i);
    }

    template<class T>
    inline TVector3D<T>& TVector3D<T>::operator/=(T scalar)
    {
        T i = (T)1.0 / scalar;
        _x *= i;
        _y *= i;
        _z *= i;

        return *this;
    }

    template<class T>
    inline bool TVector3D<T>::operator<=(const TVector3D<T>& other) const
    {
        return _x <= other._x && _y <= other._y && _z <= other._z;
    }

    template<class T>
    inline bool TVector3D<T>::operator>=(const TVector3D<T>& other) const
    {
        return _x >= other._x && _y >= other._y && _z >= other._z;
    }

    template<class T>
    inline bool TVector3D<T>::operator<(const TVector3D<T>& other) const
    {
        return _x < other._x && _y < other._y && _z < other._z;
    }

    template<class T>
    inline bool TVector3D<T>::operator>(const TVector3D<T>& other) const
    {
        return _x > other._x && _y > other._y && _z > other._z;
    }

    template<class T>
    inline bool TVector3D<T>::operator==(const TVector3D<T>& other) const
    {
        return _x == other._x && _y == other._y && _z == other._z;
    }

    template<class T>
    inline bool TVector3D<T>::operator!=(const TVector3D<T>& other) const
    {
        return _x != other._x || _y != other._y || _z != other._z;
    }

    template<class T>
    inline T TVector3D<T>::operator[](u32 index) const
    {
        return ((T*)this)[index];
    }

    template<class T>
    inline T& TVector3D<T>::operator[](u32 index)
    {
        return ((T*)this)[index];
    }

    template<class T>
    inline TVector3D<T>& TVector3D<T>::set(T x, T y, T z)
    {
        _x = x;
        _y = y;
        _z = z;

        return *this;
    }

    template<class T>
    inline TVector3D<T>& TVector3D<T>::set(const TVector3D<T>& point)
    {
        _x = point._x;
        _y = point._y;
        _z = point._z;

        return *this;
    }


    template<class T>
    inline bool TVector3D<T>::isEquals(const TVector3D<T>& other, T tolerance) const
    {
        const bool isEquels = math::isEquals(_x, other._x, tolerance) && math::isEquals(_y, other._y, tolerance) && math::isEquals(_z, other._z, tolerance);
        return isEquels;
    }

    template<class T>
    inline bool TVector3D<T>::isInLine(const TVector3D<T>& begin, const TVector3D<T>& end) const
    {
        const T f = (end - begin).lengthSQ();
        const bool isInLine = distanceFromSQ(begin) <= f && distanceFromSQ(end) <= f;
        return isInLine;
    }

    template<class T>
    inline T TVector3D<T>::length() const
    {
        const T length = (T)sqrt(((f64)_x * _x + (f64)_y * _y + (f64)_z * _z));
        return length;
    }

    template<class T>
    inline T TVector3D<T>::lengthSQ() const
    {
        return _x * _x + _y * _y + _z * _z;
    }

    template<class T>
    inline T TVector3D<T>::distanceFrom(const TVector3D<T>& other) const
    {
        return TVector3D<T>(_x - other._x, _y - other._y, _z - other._z).length();
    }

    template<class T>
    inline T TVector3D<T>::distanceFromSQ(const TVector3D<T>& other) const
    {
        return TVector3D<T>(_x - other._x, _y - other._y, _z - other._z).lengthSQ();
    }

    template<class T>
    inline TVector3D<T>& TVector3D<T>::normalize()
    {
        T l = _x * _x + _y * _y + _z * _z;
        if (l == (T)0)
        {
            return *this;
        }
        l = ((T)1) / (T)std::sqrt((T)l);
        _x *= l;
        _y *= l;
        _z *= l;

        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace math
} //namespace v3d
