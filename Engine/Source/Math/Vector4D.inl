namespace v3d
{
namespace math
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<class T>
    TVector4D<T>::TVector4D() noexcept
        : _x(0)
        , _y(0)
        , _z(0)
        , _w(0)
    {
    }

    template<class T>
    TVector4D<T>::TVector4D(T n) noexcept
        : _x(n)
        , _y(n)
        , _z(n)
        , _w(n)
    {
    }

    template<class T>
    TVector4D<T>::TVector4D(T x, T y, T z, T w) noexcept
        : _x(x)
        , _y(y)
        , _z(z)
        , _w(w)
    {
    }

    template<class T>
    TVector4D<T>::TVector4D(const TVector4D<T>& other) noexcept
        : _x(other._x)
        , _y(other._y)
        , _z(other._z)
        , _w(other._w)
    {
    }

    template<class T>
    TVector4D<T>::TVector4D(const TVector3D<T>& other, T w) noexcept
        : _x(other._x)
        , _y(other._y)
        , _z(other._z)
        , _w(w)
    {
    }

    template<class T>
    inline TVector4D<T> TVector4D<T>::operator-() const
    {
        return TVector4D<T>(-_x, -_y, -_z, -_w);
    }

    template<class T>
    inline TVector4D<T>& TVector4D<T>::operator=(const TVector4D<T>& other)
    {
        _x = other._x;
        _y = other._y;
        _z = other._z;
        _w = other._w;

        return *this;
    }

    template<class T>
    inline TVector4D<T> TVector4D<T>::operator+(const TVector4D<T>& other) const
    {
        return TVector4D<T>(_x + other._x, _y + other._y, _z + other._z, _w + other._w);
    }

    template<class T>
    inline TVector4D<T>& TVector4D<T>::operator+=(const TVector4D<T>& other)
    {
        _x += other._x;
        _y += other._y;
        _z += other._z;
        _w += other._w;

        return *this;
    }

    template<class T>
    inline TVector4D<T> TVector4D<T>::operator+(T scalar) const
    {
        return TVector4D<T>(_x + scalar, _y + scalar, _z + scalar, _w + scalar);
    }

    template<class T>
    inline TVector4D<T>& TVector4D<T>::operator+=(T scalar)
    {
        _x += scalar;
        _y += scalar;
        _z += scalar;
        _w += scalar;

        return *this;
    }

    template<class T>
    inline TVector4D<T> TVector4D<T>::operator-(const TVector4D<T>& other) const
    {
        return TVector4D<T>(_x - other._x, _y - other._y, _z - other._z, _w - other._w);
    }

    template<class T>
    inline TVector4D<T>& TVector4D<T>::operator-=(const TVector4D<T>& other)
    {
        _x -= other._x;
        _y -= other._y;
        _z -= other._z;
        _w -= other._w;

        return *this;
    }

    template<class T>
    inline TVector4D<T> TVector4D<T>::operator-(T scalar) const
    {
        return TVector4D<T>(_x - scalar, _y - scalar, _z - scalar, _w - scalar);
    }

    template<class T>
    inline TVector4D<T>& TVector4D<T>::operator-=(T scalar)
    {
        _x -= scalar;
        _y -= scalar;
        _z -= scalar;
        _w -= scalar;

        return *this;
    }

    template<class T>
    inline TVector4D<T> TVector4D<T>::operator*(const TVector4D<T>& other) const
    {
        return TVector4D<T>(_x * other._x, _y * other._y, _z * other._z, _w * other._w);
    }

    template<class T>
    inline TVector4D<T>& TVector4D<T>::operator*=(const TVector4D<T>& other)
    {
        _x *= other._x;
        _y *= other._y;
        _z *= other._z;
        _w *= other._w;

        return *this;
    }

    template<class T>
    inline TVector4D<T> TVector4D<T>::operator*(T scalar) const
    {
        return TVector4D<T>(_x * scalar, _y * scalar, _z * scalar, _w * scalar);
    }

    template<class T>
    inline TVector4D<T>& TVector4D<T>::operator*=(T scalar)
    {
        _x *= scalar;
        _y *= scalar;
        _z *= scalar;
        _w *= scalar;

        return *this;
    }

    template<class T>
    inline TVector4D<T> TVector4D<T>::operator/(const TVector4D<T>& other) const
    {
        return TVector4D<T>(_x / other._x, _y / other._y, _z / other._z, _w / other._w);
    }

    template<class T>
    inline TVector4D<T>& TVector4D<T>::operator/=(const TVector4D<T>& other)
    {
        _x /= other._x;
        _y /= other._y;
        _z /= other._z;
        _w /= other._w;

        return *this;
    }

    template<class T>
    inline TVector4D<T> TVector4D<T>::operator/(T scalar) const
    {
        T i = (T)1.0 / scalar;
        return TVector4D<T>(_x * i, _y * i, _z * i, _w * i);
    }

    template<class T>
    inline TVector4D<T>& TVector4D<T>::operator/=(T scalar)
    {
        T i = (T)1.0 / scalar;
        _x *= i;
        _y *= i;
        _z *= i;
        _w *= i;

        return *this;
    }

    template<class T>
    inline bool TVector4D<T>::operator<=(const TVector4D<T>& other) const
    {
        return _x <= other._x && _y <= other._y && _z <= other._z && _w <= other._w;
    }

    template<class T>
    inline bool TVector4D<T>::operator>=(const TVector4D<T>& other) const
    {
        return _x >= other._x && _y >= other._y && _z >= other._z && _w >= other._w;
    }

    template<class T>
    inline bool TVector4D<T>::operator<(const TVector4D<T>& other) const
    {
        return _x < other._x && _y < other._y && _z < other._z && _w < other._w;
    }

    template<class T>
    inline bool TVector4D<T>::operator>(const TVector4D<T>& other) const
    {
        return _x > other._x && _y > other._y && _z > other._z && _w > other._w;
    }

    template<class T>
    inline bool TVector4D<T>::operator==(const TVector4D<T>& other) const
    {
        return _x == other._x && _y == other._y && _z == other._z && _w == other._w;
    }

    template<class T>
    inline bool TVector4D<T>::operator!=(const TVector4D<T>& other) const
    {
        return _x != other._x || _y != other._y || _z != other._z || _w != other._w;
    }

    template<class T>
    inline T TVector4D<T>::operator[](u32 index) const
    {
        return ((T*)this)[index];
    }

    template<class T>
    inline T& TVector4D<T>::operator[](u32 index)
    {
        return ((T*)this)[index];
    }

    template<class T>
    inline bool TVector4D<T>::isEquals(const TVector4D<T>& other, T tolerance) const
    {
        const bool isEquals = math::isEquals(_x, other._x, tolerance) && math::isEquals(_y, other._y, tolerance) && math::isEquals(_z, other._z, tolerance) && math::isEquals(_w, other._w, tolerance);
        return isEquals;
    }

    template<class T>
    inline TVector4D<T>& TVector4D<T>::set(T x, T y, T z, T w)
    {
        _x = x;
        _y = y;
        _z = z;
        _w = w;

        return *this;
    }

    template<class T>
    inline TVector4D<T>& TVector4D<T>::set(const TVector4D<T>& point)
    {
        _x = point._x;
        _y = point._y;
        _z = point._z;
        _w = point._w;

        return *this;
    }

    template<class T>
    inline TVector4D<T>& TVector4D<T>::set(const TVector3D<T>& point, const T w)
    {
        _x = point._x;
        _y = point._y;
        _z = point._z;
        _w = w;

        return *this;
    }

    template<class T>
    inline T TVector4D<T>::length() const
    {
        const T length = (T)std::sqrt((f64)(_x * _x + _y * _y + _z * _z + _w * _w));
        return length;
    }

    template<class T>
    inline T TVector4D<T>::lengthSQ() const
    {
        return _x * _x + _y * _y + _z * _z + _w * _w;
    }

    template<class T>
    inline T TVector4D<T>::distanceFrom(const TVector4D<T>& other) const
    {
        const T distance = TVector4D<T>(_x - other._x, _y - other._y, _z - other._z, _w - other._w).length();
        return distance;
    }

    template<class T>
    inline T TVector4D<T>::distanceFromSQ(const TVector4D<T>& other) const
    {
        const T distanceSQ = TVector4D<T>(_x - other._x, _y - other._y, _z - other._z, _w - other._w).lengthSQ();
        return distanceSQ;
    }

    template<class T>
    inline bool TVector4D<T>::isInLine(const TVector4D<T>& begin, const TVector4D<T>& end) const
    {
        const T f = (end - begin).lengthSQ();
        const bool isInLine = distanceFromSQ(begin) <= f && distanceFromSQ(end) <= f;
        return isInLine;
    }

    template<class T>
    inline TVector4D<T>& TVector4D<T>::normalize()
    {
        T l = _x * _x + _y * _y + _z * _z + _w * _w;
        if (l == (T)0.0)
        {
            return *this;
        }
        l = ((T) 1.0) / (T)std::sqrt((T)l);
        _x *= l;
        _y *= l;
        _z *= l;
        _w *= l;

        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace math
} //namespace v3d
