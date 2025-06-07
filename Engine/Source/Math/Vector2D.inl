namespace v3d
{
namespace math
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<typename T>
    constexpr inline TVector2D<T>::TVector2D() noexcept
        : _x(0)
        , _y(0)
    {
    }

    template<typename T>
    constexpr inline TVector2D<T>::TVector2D(T x, T y) noexcept
        : _x(x)
        , _y(y)
    {
    }

    template<typename T>
    constexpr inline TVector2D<T>::TVector2D(T n) noexcept
        : _x(n)
        , _y(n)
    {
    }

    template<typename T>
    constexpr inline TVector2D<T>::TVector2D(const TVector2D<T>& other) noexcept
        : _x(other._x)
        , _y(other._y)
    {
    }

    template<typename T>
    inline TVector2D<T> TVector2D<T>::operator-() const
    {
        return TVector2D<T>(-_x, -_y);
    }

    template<typename T>
    inline TVector2D<T>& TVector2D<T>::operator=(const TVector2D<T>& other)
    {
        if (this == &other)
        {
            return *this;
        }

        _x = other._x;
        _y = other._y;

        return *this;
    }

    template<typename T>
    inline TVector2D<T> TVector2D<T>::operator+(const TVector2D<T>& other) const
    {
        return TVector2D<T>(_x + other._x, _y + other._y);
    }

    template<typename T>
    inline TVector2D<T>& TVector2D<T>::operator+=(const TVector2D<T>& other)
    {
        _x += other._x;
        _y += other._y;

        return *this;
    }

    template<typename T>
    inline TVector2D<T> TVector2D<T>::operator+(T scalar) const
    {
        return TVector2D<T>(_x + scalar, _y + scalar);
    }

    template<typename T>
    inline TVector2D<T>& TVector2D<T>::operator+=(T scalar)
    {
        _x += scalar;
        _y += scalar;

        return *this;
    }

    template<typename T>
    inline TVector2D<T> TVector2D<T>::operator-(const TVector2D<T>& other) const
    {
        return TVector2D<T>(_x - other._x, _y - other._y);
    }

    template<typename T>
    inline TVector2D<T>& TVector2D<T>::operator-=(const TVector2D<T>& other)
    {
        _x -= other._x;
        _y -= other._y;
        return *this;
    }

    template<typename T>
    inline TVector2D<T> TVector2D<T>::operator-(T scalar) const
    {
        return TVector2D<T>(_x - scalar, _y - scalar);
    }

    template<typename T>
    inline TVector2D<T>& TVector2D<T>::operator-=(T scalar)
    {
        _x -= scalar;
        _y -= scalar;

        return *this;
    }

    template<typename T>
    inline TVector2D<T> TVector2D<T>::operator*(const TVector2D<T>& other) const
    {
        return TVector2D<T>(_x * other._x, _y * other._y);
    }

    template<typename T>
    inline TVector2D<T>& TVector2D<T>::operator*=(const TVector2D<T>& other)
    {
        _x *= other._x;
        _y *= other._y;

        return *this;
    }

    template<typename T>
    inline TVector2D<T> TVector2D<T>::operator*(T scalar) const
    {
        return TVector2D<T>(_x * scalar, _y * scalar);
    }

    template<typename T>
    inline TVector2D<T>& TVector2D<T>::operator*=(T scalar)
    {
        _x *= scalar;
        _y *= scalar;

        return *this;
    }

    template<typename T>
    inline TVector2D<T> TVector2D<T>::operator/(const TVector2D<T>& other) const
    {
        return TVector2D<T>(_x / other._x, _y / other._y);
    }

    template<typename T>
    inline TVector2D<T>& TVector2D<T>::operator/=(const TVector2D<T>& other)
    {
        _x /= other._x;
        _y /= other._y;

        return *this;
    }

    template<typename T>
    inline TVector2D<T> TVector2D<T>::operator/(const T v) const
    {
        T i = (T)1.0 / v;
        TVector2D<T> out(_x * i, _y * i);

        return out;
    }

    template<typename T>
    inline TVector2D<T>& TVector2D<T>::operator/=(const T v)
    {
        T i = (T)1.0 / v;
        _x *= i;
        _y *= i;

        return *this;
    }

    template<typename T>
    inline bool TVector2D<T>::operator<=(const TVector2D<T>& other) const
    {
        return _x <= other._x && _y <= other._y;
    }

    template<typename T>
    inline bool TVector2D<T>::operator>=(const TVector2D<T>& other) const
    {
        return _x >= other._x && _y >= other._y;
    }

    template<typename T>
    inline bool TVector2D<T>::operator<(const TVector2D<T>& other) const
    {
        return _x < other._x && _y < other._y;
    }

    template<typename T>
    inline bool TVector2D<T>::operator>(const TVector2D<T>& other) const
    {
        return _x > other._x && _y > other._y;
    }

    template<typename T>
    inline bool TVector2D<T>::operator==(const TVector2D<T>& other) const
    {
        return _x == other._x && _y == other._y;
    }

    template<typename T>
    inline bool TVector2D<T>::operator!=(const TVector2D<T>& other) const
    {
        return _x != other._x || _y != other._y;
    }

    template<typename T>
    inline T TVector2D<T>::operator[](u32 index) const
    {
        return ((T*)this)[index];
    }

    template<typename T>
    inline T& TVector2D<T>::operator[](u32 index)
    {
        return ((T*)this)[index];
    }

    template<typename T>
    inline TVector2D<T>& TVector2D<T>::set(T x, T y)
    {
        _x = x;
        _y = y;

        return *this;
    }

    template<typename T>
    inline TVector2D<T>& TVector2D<T>::set(const TVector2D<T>& other)
    {
        _x = other._x;
        _y = other._y;

        return *this;
    }

    template<typename T>
    inline T TVector2D<T>::length() const
    {
        const T length = (T)std::sqrt((f64)(_x * _x + _y * _y));
        return length;
    }

    template<typename T>
    inline T TVector2D<T>::lengthSQ() const
    {
        return _x * _x + _y * _y;
    }

    template<typename T>
    inline T TVector2D<T>::distanceFrom(const TVector2D<T>& other) const
    {
        const T distance = TVector2D<T>(_x - other._x, _y - other._y).length();
        return distance;
    }

    template<typename T>
    inline T TVector2D<T>::distanceFromSQ(const TVector2D<T>& other) const
    {
        const T distanceSQ = TVector2D<T>(_x - other._x, _y - other._y).lengthSQ();
        return distanceSQ;
    }

    template<typename T>
    inline bool TVector2D<T>::isEquals(const TVector2D<T>& other, T tolerance) const
    {
        const bool isEquels = math::isEquals(_x, other._x, tolerance) && math::isEquals(_y, other._y, tolerance);
        return isEquels;
    }

    template<typename T>
    inline TVector2D<T>& TVector2D<T>::normalize()
    {
        T l = _x * _x + _y * _y;

        if (l == (T) 0.0)
        {
            return *this;
        }
        l = ((T) 1.0) / (T)std::sqrt((T)l);
        _x *= l;
        _y *= l;

        return *this;
    }

} //namespace math
} //namespace v3d
