namespace v3d
{
namespace math
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<class T>
    TQuaternion<T>::TQuaternion() noexcept
        : m_x(0)
        , m_y(0)
        , m_z(0)
        , m_w(0)
    {
    }

    template<class T>
    TQuaternion<T>::TQuaternion(const TQuaternion<T>& other) noexcept
        : m_x(other.m_x)
        , m_y(other.m_y)
        , m_z(other.m_z)
        , m_w(other.m_w)
    {
    }

    template<class T>
    TQuaternion<T>::TQuaternion(const TVector4D<T>& vector) noexcept
        : m_x(vector.m_x)
        , m_y(vector.m_y)
        , m_z(vector.m_z)
        , m_w(vector.m_w)
    {
    }

    template<class T>
    TQuaternion<T>::TQuaternion(const T nx, const T ny, const T nz, const T nw) noexcept
        : m_x(nx)
        , m_y(ny)
        , m_z(nz)
        , m_w(nw)
    {
    }

    template<class T>
    inline TQuaternion<T>& TQuaternion<T>::operator=(const TQuaternion<T>& other) const
    {
        if (this == &other)
        {
            return *this;
        }

        m_x = other.m_x;
        m_y = other.m_y;
        m_z = other.m_z;
        m_w = other.m_w;
    }

    template<class T>
    inline T TQuaternion<T>::operator[](u32 index) const
    {
        return ((T*)this)[index];
    }

    template<class T>
    inline T& TQuaternion<T>::operator[](u32 index)
    {
        return ((T*)this)[index];
    }

    template<class T>
    TQuaternion<T>& TQuaternion<T>::set(const T nx, const T ny, const T nz, const T nw)
    {
        m_x = nx;
        m_y = ny;
        m_z = nz;
        m_w = nw;
    }

    template<class T>
    TQuaternion<T>& TQuaternion<T>::set(const TVector4D<T>& vector)
    {
        m_x = vector.m_x;
        m_y = vector.m_y;
        m_z = vector.m_z;
        m_w = vector.m_w;
    }

    template<class T>
    inline bool TQuaternion<T>::operator==(const TQuaternion<T>& other) const
    {
        return m_x == other.m_x && m_y == other.m_y && m_z == other.m_z && m_w == other.m_w;
    }

    template<class T>
    inline bool TQuaternion<T>::operator!=(const TQuaternion<T>& other) const
    {
        return !(*this == other);
    }

    template<class T>
    inline TQuaternion<T> TQuaternion<T>::operator+(const TQuaternion<T>& other) const
    {
        return TQuaternion<T>(m_x + other.m_x, m_y + other.m_y, m_z + other.m_z, m_w + other.m_w);
    }

    template<class T>
    inline TQuaternion<T> TQuaternion<T>::operator+(const T scalar) const
    {
        return TQuaternion<T>(m_x + scalar, m_y + scalar, m_z + scalar, m_w + scalar);
    }

    template<class T>
    inline TQuaternion<T>& TQuaternion<T>::operator+=(const TQuaternion<T>& other)
    {
        m_x += other.m_x;
        m_y += other.m_y;
        m_z += other.m_z;
        m_w += other.m_w;

        return *this;
    }

    template<class T>
    inline TQuaternion<T>& TQuaternion<T>::operator+=(const T scalar)
    {
        m_x += scalar;
        m_y += scalar;
        m_z += scalar;
        m_w += scalar;

        return *this;
    }

    template<class T>
    inline TQuaternion<T> TQuaternion<T>::operator-(const TQuaternion<T>& other) const
    {
        return TQuaternion<T>(m_x - other.m_x, m_y - other.m_y, m_z - other.m_z, m_w - other.m_w);
    }

    template<class T>
    inline TQuaternion<T> TQuaternion<T>::operator-(const T scalar) const
    {
        return TQuaternion<T>(m_x - scalar, m_y - scalar, m_z - scalar, m_w - scalar);
    }

    template<class T>
    inline TQuaternion<T>& TQuaternion<T>::operator-=(const TQuaternion<T>& other)
    {
        m_x -= other.m_x;
        m_y -= other.m_y;
        m_z -= other.m_z;
        m_w -= other.m_w;

        return *this;
    }

    template<class T>
    inline TQuaternion<T>& TQuaternion<T>::operator-=(const T scalar)
    {
        m_x -= scalar;
        m_y -= scalar;
        m_z -= scalar;
        m_w -= scalar;

        return *this;
    }

    template<class T>
    inline TQuaternion<T> TQuaternion<T>::operator*(const TQuaternion<T>& other) const
    {
        T tempX = m_w * other.m_x + m_x * other.m_w + m_y * other.m_z - m_z * other.m_y;
        T tempY = m_w * other.m_y + m_y * other.m_w + m_z * other.m_x - m_x * other.m_z;
        T tempZ = m_w * other.m_z + m_z * other.m_w + m_x * other.m_y - m_y * other.m_x;
        T tempW = m_w * other.m_w - m_x * other.m_x - m_y * other.m_y - m_z * other.m_z;

        return TQuaternion<T>(tempX, tempY, tempZ, tempW);
    }

    template<class T>
    inline TQuaternion<T> TQuaternion<T>::operator*(const T scalar) const
    {
        return TQuaternion<T>(m_x * scalar, m_y * scalar, m_z * scalar, m_w * scalar);
    }

    template<class T>
    inline TQuaternion<T>& TQuaternion<T>::operator*=(const TQuaternion<T>& other)
    {
        T tempX = m_w * other.m_x + m_x * other.m_w + m_y * other.m_z - m_z * other.m_y;
        T tempY = m_w * other.m_y + m_y * other.m_w + m_z * other.m_x - m_x * other.m_z;
        T tempZ = m_w * other.m_z + m_z * other.m_w + m_x * other.m_y - m_y * other.m_x;
        T tempW = m_w * other.m_w - m_x * other.m_x - m_y * other.m_y - m_z * other.m_z;

        m_x = tempX;
        m_y = tempY;
        m_z = tempZ;
        m_w = tempW;

        return *this;
    }

    template<class T>
    inline TQuaternion<T>& TQuaternion<T>::operator*=(const T scalar)
    {
        m_x *= scalar;
        m_y *= scalar;
        m_z *= scalar;
        m_w *= scalar;

        return *this;
    }

    template<class T>
    inline TQuaternion<T> TQuaternion<T>::operator/(const T scalar) const
    {
        return TQuaternion<T>(m_x / scalar, m_y / scalar, m_z / scalar, m_w / scalar);
    }

    template<class T>
    inline TQuaternion<T>& TQuaternion<T>::operator/=(const T scalar)
    {
        m_x /= scalar;
        m_y /= scalar;
        m_z /= scalar;
        m_w /= scalar;

        return *this;
    }

    template<class T>
    inline TQuaternion<T> TQuaternion<T>::operator-() const
    {
        return TQuaternion<T>(-m_x, -m_y, -m_z, -m_w);
    }

    template<class T>
    inline void TQuaternion<T>::makeIdentity()
    {
        m_x = (T)0;
        m_y = (T)0;
        m_z = (T)0;
        m_w = (T)0;
    }

    template<class T>
    inline bool TQuaternion<T>::isIdentity() const
    {
        return math::isZero(m_x) && math::isZero(m_y) && math::isZero(m_z) && math::isZero(m_w);
    }

    template<class T>
    inline TQuaternion<T>& TQuaternion<T>::invert()
    {
        m_x = -m_x;
        m_y = -m_y;
        m_z = -m_z;
        m_w = -m_w;

        return *this;
    }

    template<class T>
    inline TQuaternion<T>& TQuaternion<T>::normalize()
    {
        T l = dotProduct<T>(*this, *this);
        if (l == (T)0)
        {
            return *this;
        }

        T l = ((T)1) / (T)sqrt((T)l);
        *this *= l;

        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace math
} //namespace v3d