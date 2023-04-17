namespace v3d
{
namespace math
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<class T>
    TVector2D<T>::TVector2D() noexcept
        : m_x(0)
        , m_y(0)
    {}

    template<class T>
    TVector2D<T>::TVector2D(T nx, T ny) noexcept
        : m_x(nx)
        , m_y(ny)
    {}

    template<class T>
    TVector2D<T>::TVector2D(T n) noexcept
        : m_x(n)
        , m_y(n)
    {}

    template<class T>
    TVector2D<T>::TVector2D(const TVector2D<T>& other) noexcept
        : m_x(other.m_x)
        , m_y(other.m_y)
    {}

    template<class T>
    inline TVector2D<T> TVector2D<T>::operator-() const
    {
        return TVector2D<T>(-m_x, -m_y);
    }

    template<class T>
    inline TVector2D<T>& TVector2D<T>::operator=(const TVector2D<T>& other)
    {
        if (this == &other)
        {
            return *this;
        }

        m_x = other.m_x;
        m_y = other.m_y;

        return *this;
    }

    template<class T>
    inline TVector2D<T>& TVector2D<T>::operator=(const T* other)
    {
        m_x = other[0];
        m_y = other[1];

        return *this;
    }

    template<class T>
    inline TVector2D<T> TVector2D<T>::operator+(const TVector2D<T>& other) const
    {
        return TVector2D<T>(m_x + other.m_x, m_y + other.m_y);
    }

    template<class T>
    inline TVector2D<T>& TVector2D<T>::operator+=(const TVector2D<T>& other)
    {
        m_x += other.m_x;
        m_y += other.m_y;

        return *this;
    }

    template<class T>
    inline TVector2D<T> TVector2D<T>::operator+(const T scalar) const
    {
        return TVector2D<T>(m_x + scalar, m_y + scalar);
    }

    template<class T>
    inline TVector2D<T>& TVector2D<T>::operator+=(const T scalar)
    {
        m_x += scalar;
        m_y += scalar;

        return *this;
    }

    template<class T>
    inline TVector2D<T> TVector2D<T>::operator-(const TVector2D<T>& other) const
    {
        return TVector2D<T>(m_x - other.m_x, m_y - other.m_y);
    }

    template<class T>
    inline TVector2D<T>& TVector2D<T>::operator-=(const TVector2D<T>& other)
    {
        m_x -= other.m_x;
        m_y -= other.m_y;
        return *this;
    }

    template<class T>
    inline TVector2D<T> TVector2D<T>::operator-(const T scalar) const
    {
        return TVector2D<T>(m_x - scalar, m_y - scalar);
    }

    template<class T>
    inline TVector2D<T>& TVector2D<T>::operator-=(const T scalar)
    {
        m_x -= scalar;
        m_y -= scalar;

        return *this;
    }

    template<class T>
    inline TVector2D<T> TVector2D<T>::operator*(const TVector2D<T>& other) const
    {
        return TVector2D<T>(m_x * other.m_x, m_y * other.m_y);
    }

    template<class T>
    inline TVector2D<T>& TVector2D<T>::operator*=(const TVector2D<T>& other)
    {
        m_x *= other.m_x;
        m_y *= other.m_y;

        return *this;
    }

    template<class T>
    inline TVector2D<T> TVector2D<T>::operator*(const T scalar) const
    {
        return TVector2D<T>(m_x * scalar, m_y * scalar);
    }

    template<class T>
    inline TVector2D<T>& TVector2D<T>::operator*=(const T scalar)
    {
        m_x *= scalar;
        m_y *= scalar;

        return *this;
    }

    template<class T>
    inline TVector2D<T> TVector2D<T>::operator/(const TVector2D<T>& other) const
    {
        return TVector2D<T>(m_x / other.m_x, m_y / other.m_y);
    }

    template<class T>
    inline TVector2D<T>& TVector2D<T>::operator/=(const TVector2D<T>& other)
    {
        m_x /= other.m_x;
        m_y /= other.m_y;

        return *this;
    }

    template<class T>
    inline TVector2D<T> TVector2D<T>::operator/(const T v) const
    {
        T i = (T)1.0 / v;
        TVector2D<T> out(m_x * i, m_y * i);

        return out;
    }

    template<class T>
    inline TVector2D<T>& TVector2D<T>::operator/=(const T v)
    {
        T i = (T)1.0 / v;
        m_x *= i;
        m_y *= i;

        return *this;
    }

    template<class T>
    inline bool TVector2D<T>::operator<=(const TVector2D<T>& other) const
    {
        return m_x <= other.m_x && m_y <= other.m_y;
    }

    template<class T>
    inline bool TVector2D<T>::operator>=(const TVector2D<T>& other) const
    {
        return m_x >= other.m_x && m_y >= other.m_y;
    }

    template<class T>
    inline bool TVector2D<T>::operator<(const TVector2D<T>& other) const
    {
        return m_x < other.m_x && m_y < other.m_y;
    }

    template<class T>
    inline bool TVector2D<T>::operator>(const TVector2D<T>& other) const
    {
        return m_x > other.m_x && m_y > other.m_y;
    }

    template<class T>
    inline bool TVector2D<T>::operator==(const TVector2D<T>& other) const
    {
        return this->isEquals(other);
    }

    template<class T>
    inline bool TVector2D<T>::operator!=(const TVector2D<T>& other) const
    {
        return !this->isEquals(other);
    }

    template<class T>
    inline T TVector2D<T>::operator[](u32 index) const
    {
        return ((T*)this)[index];
    }

    template<class T>
    inline T& TVector2D<T>::operator[](u32 index)
    {
        return ((T*)this)[index];
    }

    template<class T>
    inline TVector2D<T>& TVector2D<T>::set(const T nx, const T ny)
    {
        m_x = nx;
        m_y = ny;

        return *this;
    }

    template<class T>
    inline TVector2D<T>& TVector2D<T>::set(const TVector2D<T>& other)
    {
        m_x = other.m_x;
        m_y = other.m_y;

        return *this;
    }

    template<class T>
    inline T TVector2D<T>::length() const
    {
        const T length = (T)sqrt((f64)(m_x * m_x + m_y * m_y));
        return length;
    }

    template<class T>
    inline T TVector2D<T>::lengthSQ() const
    {
        return m_x * m_x + m_y * m_y;
    }

    template<class T>
    inline T TVector2D<T>::distanceFrom(const TVector2D<T>& other) const
    {
        const T distance = TVector2D<T>(m_x - other.m_x, m_y - other.m_y).length();
        return distance;
    }

    template<class T>
    inline T TVector2D<T>::distanceFromSQ(const TVector2D<T>& other) const
    {
        const T distanceSQ = TVector2D<T>(m_x - other.m_x, m_y - other.m_y).lengthSQ();
        return distanceSQ;
    }

    template<class T>
    inline bool TVector2D<T>::isEquals(const TVector2D<T>& other, const T tolerance) const
    {
        const bool isEquels = math::isEquals(m_x, other.m_x, tolerance) && math::isEquals(m_y, other.m_y, tolerance);
        return isEquels;
    }

    template<class T>
    inline bool TVector2D<T>::isInLine(const TVector2D<T>& begin, const TVector2D<T>& end) const
    {
        const T f = (end - begin).lengthSQ();
        const bool isInLine = distanceFromSQ(begin) <= f && distanceFromSQ(end) <= f;
        return isInLine;
    }

    template<class T>
    inline TVector2D<T>& TVector2D<T>::normalize()
    {
        T l = m_x * m_x + m_y * m_y;

        if (l == (T) 0.0)
        {
            return *this;
        }
        l = ((T) 1.0) / (T)sqrt((T)l);
        m_x *= l;
        m_y *= l;

        return *this;
    }

    template<class T>
    inline TVector2D<T>& TVector2D<T>::invert()
    {
        m_x *= -m_x;
        m_y *= -m_y;

        return *this;
    }

    template<class T>
    inline TVector2D<T>& TVector2D<T>::setLength(T newlength)
    {
        normalize();
        return (*this *= newlength);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace math
} //namespace v3d
