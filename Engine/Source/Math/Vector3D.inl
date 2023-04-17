namespace v3d
{
namespace math
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<class T>
    TVector3D<T>::TVector3D() noexcept
        : m_x(0)
        , m_y(0)
        , m_z(0)
    {}

    template<class T>
    TVector3D<T>::TVector3D(T n) noexcept
        : m_x(n)
        , m_y(n)
        , m_z(n)
    {}

    template<class T>
    TVector3D<T>::TVector3D(T nx, T ny, T nz) noexcept
        : m_x(nx)
        , m_y(ny)
        , m_z(nz)
    {}

    template<class T>
    TVector3D<T>::TVector3D(const TVector3D<T>& other) noexcept
        : m_x(other.m_x)
        , m_y(other.m_y)
        , m_z(other.m_z)
    {}

    template<class T>
    TVector3D<T>::TVector3D(const TVector2D<T>& other, T nz) noexcept
        : m_x(other.m_x)
        , m_y(other.m_y)
        , m_z(nz)
    {}

    template<class T>
    inline TVector3D<T> TVector3D<T>::operator-() const
    {
        return TVector3D<T>(-m_x, -m_y, -m_z);
    }

    template<class T>
    inline TVector3D<T>& TVector3D<T>::operator=(const TVector3D<T>& other)
    {
        if (this == &other)
        {
            return *this;
        }

        m_x = other.m_x;
        m_y = other.m_y;
        m_z = other.m_z;

        return *this;
    }

    template<class T>
    inline TVector3D<T>& TVector3D<T>::operator=(const T* other)
    {
        m_x = other[0];
        m_y = other[1];
        m_z = other[2];

        return *this;
    }

    template<class T>
    inline TVector3D<T> TVector3D<T>::operator+(const TVector3D<T>& other) const
    {
        return TVector3D<T>(m_x + other.m_x, m_y + other.m_y, m_z + other.m_z);
    }

    template<class T>
    inline TVector3D<T>& TVector3D<T>::operator+=(const TVector3D<T>& other)
    {
        m_x += other.m_x;
        m_y += other.m_y;
        m_z += other.m_z;

        return *this;
    }

    template<class T>
    inline TVector3D<T> TVector3D<T>::operator+(const T scalar) const
    {
        return TVector3D<T>(m_x + scalar, m_y + scalar, m_z + scalar);
    }

    template<class T>
    inline TVector3D<T>& TVector3D<T>::operator+=(const T scalar)
    {
        m_x += scalar;
        m_y += scalar;
        m_z += scalar;

        return *this;
    }

    template<class T>
    inline TVector3D<T> TVector3D<T>::operator-(const TVector3D<T>& other) const
    {
        return TVector3D<T>(m_x - other.m_x, m_y - other.m_y, m_z - other.m_z);
    }

    template<class T>
    inline TVector3D<T>& TVector3D<T>::operator-=(const TVector3D<T>& other)
    {
        m_x -= other.m_x;
        m_y -= other.m_y;
        m_z -= other.m_z;

        return *this;
    }

    template<class T>
    inline TVector3D<T> TVector3D<T>::operator-(const T scalar) const
    {
        return TVector3D<T>(m_x - scalar, m_y - scalar, m_z - scalar);
    }

    template<class T>
    inline TVector3D<T>& TVector3D<T>::operator-=(const T scalar)
    {
        m_x -= scalar;
        m_y -= scalar;
        m_z -= scalar;

        return *this;
    }

    template<class T>
    inline TVector3D<T> TVector3D<T>::operator*(const TVector3D<T>& other) const
    {
        return TVector3D<T>(m_x * other.m_x, m_y * other.m_y, m_z * other.m_z);
    }

    template<class T>
    inline TVector3D<T>& TVector3D<T>::operator*=(const TVector3D<T>& other)
    {
        m_x *= other.m_x;
        m_y *= other.m_y;
        m_z *= other.m_z;

        return *this;
    }

    template<class T>
    inline TVector3D<T> TVector3D<T>::operator*(const T scalar) const
    {
        return TVector3D<T>(m_x * scalar, m_y * scalar, m_z * scalar);
    }

    template<class T>
    inline TVector3D<T>& TVector3D<T>::operator*=(const T scalar)
    {
        m_x *= scalar;
        m_y *= scalar;
        m_z *= scalar;

        return *this;
    }

    template<class T>
    inline TVector3D<T> TVector3D<T>::operator/(const TVector3D<T>& other) const
    {
        return TVector3D<T>(m_x / other.m_x, m_y / other.m_y, m_z / other.m_z);
    }

    template<class T>
    inline TVector3D<T>& TVector3D<T>::operator/=(const TVector3D<T>& other)
    {
        m_x /= other.m_x;
        m_y /= other.m_y;
        m_z /= other.m_z;

        return *this;
    }

    template<class T>
    inline TVector3D<T> TVector3D<T>::operator/(const T scalar) const
    {
        T i = (T)1.0 / scalar;
        return TVector3D<T>(m_x * i, m_y * i, m_z * i);
    }

    template<class T>
    inline TVector3D<T>& TVector3D<T>::operator/=(const T scalar)
    {
        T i = (T)1.0 / scalar;
        m_x *= i;
        m_y *= i;
        m_z *= i;

        return *this;
    }

    template<class T>
    inline bool TVector3D<T>::operator<=(const TVector3D<T>& other) const
    {
        return m_x <= other.m_x && m_y <= other.m_y && m_z <= other.m_z;
    }

    template<class T>
    inline bool TVector3D<T>::operator>=(const TVector3D<T>& other) const
    {
        return m_x >= other.m_x && m_y >= other.m_y && m_z >= other.m_z;
    }

    template<class T>
    inline bool TVector3D<T>::operator<(const TVector3D<T>& other) const
    {
        return m_x < other.m_x && m_y < other.m_y && m_z < other.m_z;
    }

    template<class T>
    inline bool TVector3D<T>::operator>(const TVector3D<T>& other) const
    {
        return m_x > other.m_x && m_y > other.m_y && m_z > other.m_z;
    }

    template<class T>
    inline bool TVector3D<T>::operator==(const TVector3D<T>& other) const
    {
        return this->isEquals(other);
    }

    template<class T>
    inline bool TVector3D<T>::operator!=(const TVector3D<T>& other) const
    {
        return !this->isEquals(other);
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
    inline TVector3D<T>& TVector3D<T>::set(const T nx, const T ny, const T nz)
    {
        m_x = nx;
        m_y = ny;
        m_z = nz;

        return *this;
    }

    template<class T>
    inline TVector3D<T>& TVector3D<T>::set(const TVector3D<T>& point)
    {
        m_x = point.m_x;
        m_y = point.m_y;
        m_z = point.m_z;

        return *this;
    }


    template<class T>
    inline bool TVector3D<T>::isEquals(const TVector3D<T>& other, const T tolerance) const
    {
        const bool isEquels = math::isEquals(m_x, other.m_x, tolerance) &&
            math::isEquals(m_y, other.m_y, tolerance) &&
            math::isEquals(m_z, other.m_z, tolerance);
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
        const T length = (T)sqrt(((f64)m_x * m_x + (f64)m_y * m_y + (f64)m_z * m_z));
        return length;
    }

    template<class T>
    inline T TVector3D<T>::lengthSQ() const
    {
        return m_x * m_x + m_y * m_y + m_z * m_z;
    }

    template<class T>
    inline T TVector3D<T>::distanceFrom(const TVector3D<T>& other) const
    {
        return TVector3D<T>(m_x - other.m_x, m_y - other.m_y, m_z - other.m_z).length();
    }

    template<class T>
    inline T TVector3D<T>::distanceFromSQ(const TVector3D<T>& other) const
    {
        return TVector3D<T>(m_x - other.m_x, m_y - other.m_y, m_z - other.m_z).lengthSQ();
    }

    template<class T>
    inline TVector3D<T>& TVector3D<T>::normalize()
    {
        T l = m_x * m_x + m_y * m_y + m_z * m_z;
        if (l == (T)0.0)
        {
            return *this;
        }
        l = ((T) 1.0) / (T)sqrt((T)l);
        m_x *= l;
        m_y *= l;
        m_z *= l;

        return *this;
    }

    template<class T>
    inline TVector3D<T>& TVector3D<T>::invert()
    {
        m_x *= -m_x;
        m_y *= -m_y;
        m_z *= -m_z;

        return *this;
    }

    template<class T>
    inline TVector3D<T>& TVector3D<T>::setLength(T newlength)
    {
        normalize();
        return (*this *= newlength);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace math
} //namespace v3d
