namespace v3d
{
namespace math
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<class T>
    TVector4D<T>::TVector4D() noexcept
        : m_x(0)
        , m_y(0)
        , m_z(0)
        , m_w(0)
    {}

    template<class T>
    TVector4D<T>::TVector4D(T n) noexcept
        : m_x(n)
        , m_y(n)
        , m_z(n)
        , m_w(n)
    {}

    template<class T>
    TVector4D<T>::TVector4D(T nx, T nw) noexcept
        : m_x(nx)
        , m_y(nx)
        , m_z(nx)
        , m_w(nw)
    {}

    template<class T>
    TVector4D<T>::TVector4D(T nx, T ny, T nz, T nw) noexcept
        : m_x(nx)
        , m_y(ny)
        , m_z(nz)
        , m_w(nw)
    {}

    template<class T>
    TVector4D<T>::TVector4D(const TVector4D<T>& other) noexcept
        : m_x(other.m_x)
        , m_y(other.m_y)
        , m_z(other.m_z)
        , m_w(other.m_w)
    {}

    template<class T>
    TVector4D<T>::TVector4D(const TVector3D<T>& other, T nw) noexcept
        : m_x(other.m_x)
        , m_y(other.m_y)
        , m_z(other.m_z)
        , m_w(nw)
    {}

    template<class T>
    inline TVector4D<T> TVector4D<T>::operator-() const
    {
        return TVector4D<T>(-m_x, -m_y, -m_z, -m_w);
    }

    template<class T>
    inline TVector4D<T>& TVector4D<T>::operator=(const TVector4D<T>& other)
    {
        m_x = other.m_x;
        m_y = other.m_y;
        m_z = other.m_z;
        m_w = other.m_w;

        return *this;
    }

    template<class T>
    inline TVector4D<T> TVector4D<T>::operator+(const TVector4D<T>& other) const
    {
        return TVector4D<T>(m_x + other.m_x, m_y + other.m_y, m_z + other.m_z, m_w + other.m_w);
    }

    template<class T>
    inline TVector4D<T>& TVector4D<T>::operator+=(const TVector4D<T>& other)
    {
        m_x += other.m_x;
        m_y += other.m_y;
        m_z += other.m_z;
        m_w += other.m_w;

        return *this;
    }

    template<class T>
    inline TVector4D<T> TVector4D<T>::operator+(const T scalar) const
    {
        return TVector4D<T>(m_x + scalar, m_y + scalar, m_z + scalar, m_w + scalar);
    }

    template<class T>
    inline TVector4D<T>& TVector4D<T>::operator+=(const T scalar)
    {
        m_x += scalar;
        m_y += scalar;
        m_z += scalar;
        m_w += scalar;

        return *this;
    }

    template<class T>
    inline TVector4D<T> TVector4D<T>::operator-(const TVector4D<T>& other) const
    {
        return TVector4D<T>(m_x - other.m_x, m_y - other.m_y, m_z - other.m_z, m_w - other.m_w);
    }

    template<class T>
    inline TVector4D<T>& TVector4D<T>::operator-=(const TVector4D<T>& other)
    {
        m_x -= other.m_x;
        m_y -= other.m_y;
        m_z -= other.m_z;
        m_w -= other.m_w;

        return *this;
    }

    template<class T>
    inline TVector4D<T> TVector4D<T>::operator-(const T scalar) const
    {
        return TVector4D<T>(m_x - scalar, m_y - scalar, m_z - scalar, m_w - scalar);
    }

    template<class T>
    inline TVector4D<T>& TVector4D<T>::operator-=(const T scalar)
    {
        m_x -= scalar;
        m_y -= scalar;
        m_z -= scalar;
        m_w -= scalar;

        return *this;
    }

    template<class T>
    inline TVector4D<T> TVector4D<T>::operator*(const TVector4D<T>& other) const
    {
        return TVector4D<T>(m_x * other.m_x, m_y * other.m_y, m_z * other.m_z, m_w * other.m_w);
    }

    template<class T>
    inline TVector4D<T>& TVector4D<T>::operator*=(const TVector4D<T>& other)
    {
        m_x *= other.m_x;
        m_y *= other.m_y;
        m_z *= other.m_z;
        m_w *= other.m_w;

        return *this;
    }

    template<class T>
    inline TVector4D<T> TVector4D<T>::operator*(const T scalar) const
    {
        return TVector4D<T>(m_x * scalar, m_y * scalar, m_z * scalar, m_w * scalar);
    }

    template<class T>
    inline TVector4D<T>& TVector4D<T>::operator*=(const T scalar)
    {
        m_x *= scalar;
        m_y *= scalar;
        m_z *= scalar;
        m_w *= scalar;

        return *this;
    }

    template<class T>
    inline TVector4D<T> TVector4D<T>::operator/(const TVector4D<T>& other) const
    {
        return TVector4D<T>(m_x / other.m_x, m_y / other.m_y, m_z / other.m_z, m_w / other.m_w);
    }

    template<class T>
    inline TVector4D<T>& TVector4D<T>::operator/=(const TVector4D<T>& other)
    {
        m_x /= other.m_x;
        m_y /= other.m_y;
        m_z /= other.m_z;
        m_w /= other.m_w;

        return *this;
    }

    template<class T>
    inline TVector4D<T> TVector4D<T>::operator/(const T scalar) const
    {
        T i = (T)1.0 / scalar;
        return TVector4D<T>(m_x * i, m_y * i, m_z * i, m_w * i);
    }

    template<class T>
    inline TVector4D<T>& TVector4D<T>::operator/=(const T scalar)
    {
        T i = (T)1.0 / scalar;
        m_x *= i;
        m_y *= i;
        m_z *= i;
        m_w *= i;

        return *this;
    }

    template<class T>
    inline bool TVector4D<T>::operator<=(const TVector4D<T>& other) const
    {
        return m_x <= other.m_x && m_y <= other.m_y && m_z <= other.m_z && m_w <= other.m_w;
    }

    template<class T>
    inline bool TVector4D<T>::operator>=(const TVector4D<T>& other) const
    {
        return m_x >= other.m_x && m_y >= other.m_y && m_z >= other.m_z && m_w >= other.m_w;
    }

    template<class T>
    inline bool TVector4D<T>::operator<(const TVector4D<T>& other) const
    {
        return m_x < other.m_x && m_y < other.m_y && m_z < other.m_z && m_w < other.m_w;
    }

    template<class T>
    inline bool TVector4D<T>::operator>(const TVector4D<T>& other) const
    {
        return m_x > other.m_x && m_y > other.m_y && m_z > other.m_z && m_w > other.m_w;
    }

    template<class T>
    inline bool TVector4D<T>::operator==(const TVector4D<T>& other) const
    {
        return this->isEquals(other);
    }

    template<class T>
    inline bool TVector4D<T>::operator!=(const TVector4D<T>& other) const
    {
        return !this->isEquals(other);
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
    inline bool TVector4D<T>::isEquals(const TVector4D<T>& other, const T tolerance) const
    {
        const bool isEquals = math::isEquals(m_x, other.m_x, tolerance) &&
            math::isEquals(m_y, other.m_y, tolerance) &&
            math::isEquals(m_z, other.m_z, tolerance) &&
            math::isEquals(m_w, other.m_w, tolerance);
        return isEquals;
    }

    template<class T>
    inline TVector4D<T>& TVector4D<T>::set(const T nx, const T ny, const T nz, const T nw)
    {
        m_x = nx;
        m_y = ny;
        m_z = nz;
        m_w = nw;

        return *this;
    }

    template<class T>
    inline TVector4D<T>& TVector4D<T>::set(const TVector4D<T>& point)
    {
        m_x = point.m_x;
        m_y = point.m_y;
        m_z = point.m_z;
        m_w = point.m_w;

        return *this;
    }

    template<class T>
    inline TVector4D<T>& TVector4D<T>::set(const TVector3D<T>& point, const T nw)
    {
        m_x = point.m_x;
        m_y = point.m_y;
        m_z = point.m_z;
        m_w = nw;

        return *this;
    }

    template<class T>
    inline T TVector4D<T>::length() const
    {
        const T length = (T)sqrt((f64)(m_x * m_x + m_y * m_y + m_z * m_z + m_w * m_w));
        return length;
    }

    template<class T>
    inline T TVector4D<T>::lengthSQ() const
    {
        return m_x * m_x + m_y * m_y + m_z * m_z + m_w * m_w;
    }

    template<class T>
    inline T TVector4D<T>::distanceFrom(const TVector4D<T>& other) const
    {
        const T distance = TVector4D<T>(m_x - other.m_x, m_y - other.m_y, m_z - other.m_z, m_w - other.m_w).length();
        return distance;
    }

    template<class T>
    inline T TVector4D<T>::distanceFromSQ(const TVector4D<T>& other) const
    {
        const T distanceSQ = TVector4D<T>(m_x - other.m_x, m_y - other.m_y, m_z - other.m_z, m_w - other.m_w).lengthSQ();
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
        T l = m_x * m_x + m_y * m_y + m_z * m_z + m_w * m_w;
        if (l == (T)0.0)
        {
            return *this;
        }
        l = ((T) 1.0) / (T)sqrt((T)l);
        m_x *= l;
        m_y *= l;
        m_z *= l;
        m_w *= l;

        return *this;
    }

    template<class T>
    inline TVector4D<T>& TVector4D<T>::setLength(T newlength)
    {
        normalize();
        return (*this *= newlength);
    }

    template<class T>
    inline TVector4D<T>& TVector4D<T>::invert()
    {
        m_x *= -m_x;
        m_y *= -m_y;
        m_z *= -m_z;
        m_w *= -m_w;

        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace math
} //namespace v3d
