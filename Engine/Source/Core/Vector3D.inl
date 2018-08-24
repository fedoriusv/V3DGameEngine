namespace v3d
{
namespace core
{

    template<class T>
    TVector3D<T>::TVector3D()
        : x(0)
        , y(0)
        , z(0)
    {}

    template<class T>
    TVector3D<T>::TVector3D(T nx)
        : x(nx)
        , y(nx)
        , z(nx)
    {}

    template<class T>
    TVector3D<T>::TVector3D(T nx, T ny, T nz)
        : x(nx)
        , y(ny)
        , z(nz)
    {}

    template<class T>
    TVector3D<T>::TVector3D(const TVector3D<T>& other)
        : x(other.x)
        , y(other.y)
        , z(other.z)
    {}

    template<class T>
    TVector3D<T>::TVector3D(const TVector2D<T>& other, T nz)
        : x(other.x)
        , y(other.y)
        , z(nz)
    {}

    template<class T>
    TVector3D<T> TVector3D<T>::operator - () const
    {
        return TVector3D<T>(-x, -y, -z);
    }

    template<class T>
    TVector3D<T>& TVector3D<T>::operator = (const TVector3D<T>& other)
    {
        if (this == &other)
        {
            return *this;
        }

        x = other.x;
        y = other.y;
        z = other.z;

        return *this;
    }

    template<class T>
    TVector3D<T>& TVector3D<T>::operator = (const T* other)
    {
        x = other[0];
        y = other[1];
        z = other[2];

        return *this;
    }

    template<class T>
    TVector3D<T> TVector3D<T>::operator + (const TVector3D<T>& other) const
    {
        return TVector3D<T>(x + other.x, y + other.y, z + other.z);
    }

    template<class T>
    TVector3D<T>& TVector3D<T>::operator += (const TVector3D<T>& other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    template<class T>
    TVector3D<T> TVector3D<T>::operator + (const T scalar) const
    {
        return TVector3D<T>(x + scalar, y + scalar, z + scalar);
    }

    template<class T>
    TVector3D<T>& TVector3D<T>::operator += (const T scalar)
    {
        x += scalar;
        y += scalar;
        z += scalar;
        return *this;
    }

    template<class T>
    TVector3D<T> TVector3D<T>::operator - (const TVector3D<T>& other) const
    {
        return TVector3D<T>(x - other.x, y - other.y, z - other.z);
    }

    template<class T>
    TVector3D<T>& TVector3D<T>::operator -= (const TVector3D<T>& other)
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    template<class T>
    TVector3D<T> TVector3D<T>::operator - (const T scalar) const
    {
        return TVector3D<T>(x - scalar, y - scalar, z - scalar);
    }

    template<class T>
    TVector3D<T>& TVector3D<T>::operator -= (const T scalar)
    {
        x -= scalar;
        y -= scalar;
        z -= scalar;
        return *this;
    }

    template<class T>
    TVector3D<T> TVector3D<T>::operator * (const TVector3D<T>& other) const
    {
        return TVector3D<T>(x * other.x, y * other.y, z * other.z);
    }

    template<class T>
    TVector3D<T>& TVector3D<T>::operator *= (const TVector3D<T>& other)
    {
        x *= other.x;
        y *= other.y;
        z *= other.z;
        return *this;
    }

    template<class T>
    TVector3D<T> TVector3D<T>::operator * (const T scalar) const
    {
        return TVector3D<T>(x * scalar, y * scalar, z * scalar);
    }

    template<class T>
    TVector3D<T>& TVector3D<T>::operator *= (const T scalar)
    {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    template<class T>
    TVector3D<T> TVector3D<T>::operator / (const TVector3D<T>& other) const
    {
        return TVector3D<T>(x / other.x, y / other.y, z / other.z);
    }

    template<class T>
    TVector3D<T>& TVector3D<T>::operator /= (const TVector3D<T>& other)
    {
        x /= other.x;
        y /= other.y;
        z /= other.z;
        return *this;
    }

    template<class T>
    TVector3D<T> TVector3D<T>::operator / (const T scalar) const
    {
        T i = (T)1.0 / scalar;
        return TVector3D<T>(x * i, y * i, z * i);
    }

    template<class T>
    TVector3D<T>& TVector3D<T>::operator /= (const T scalar)
    {
        T i = (T)1.0 / scalar;
        x *= i;
        y *= i;
        z *= i;
        return *this;
    }

    template<class T>
    bool TVector3D<T>::operator <= (const TVector3D<T>&other) const
    {
        return x <= other.x && y <= other.y && z <= other.z;
    }

    template<class T>
    bool TVector3D<T>::operator >= (const TVector3D<T>&other) const
    {
        return x >= other.x && y >= other.y && z >= other.z;
    }

    template<class T>
    bool TVector3D<T>::operator < (const TVector3D<T>&other) const
    {
        return x < other.x && y < other.y && z < other.z;
    }

    template<class T>
    bool TVector3D<T>::operator >(const TVector3D<T>&other) const
    {
        return x > other.x && y > other.y && z > other.z;
    }

    template<class T>
    bool TVector3D<T>::operator == (const TVector3D<T>& other) const
    {
        return this->isEquals(other);
    }

    template<class T>
    bool TVector3D<T>::operator!=(const TVector3D<T>& other) const
    {
        return !this->isEquals(other);
    }

    template<class T>
    T TVector3D<T>::operator [] (u32 index) const
    {
        return ((T*)this)[index];
    }

    template<class T>
    T& TVector3D<T>::operator [] (u32 index)
    {
        return ((T*)this)[index];
    }

    template<class T>
    TVector3D<T>& TVector3D<T>::set(const T nx, const T ny, const T nz)
    {
        x = nx;
        y = ny;
        z = nz;
        return *this;
    }

    template<class T>
    TVector3D<T>& TVector3D<T>::set(const TVector3D<T>& point)
    {
        x = point.x;
        y = point.y;
        z = point.z;
        return *this;
    }


    template<class T>
    bool TVector3D<T>::isEquals(const TVector3D<T>& other, const T tolerance) const
    {
        const bool isEquels = core::isEquals(x, other.x, tolerance) &&
            core::isEquals(y, other.y, tolerance) &&
            core::isEquals(z, other.z, tolerance);
        return isEquels;
    }

    template<class T>
    bool TVector3D<T>::isInLine(const TVector3D<T>& begin, const TVector3D<T>& end) const
    {
        const T f = (end - begin).lengthSQ();
        const bool isInLine = distanceFromSQ(begin) <= f && distanceFromSQ(end) <= f;
        return isInLine;
    }

    template<class T>
    T TVector3D<T>::length() const
    {
        const T length = (T)sqrt(((f64)x * x + (f64)y * y + (f64)z * z));
        return length;
    }

    template<class T>
    T TVector3D<T>::lengthSQ() const
    {
        return x * x + y * y + z * z;
    }

    template<class T>
    T TVector3D<T>::distanceFrom(const TVector3D<T>& other) const
    {
        return TVector3D<T>(x - other.x, y - other.y, z - other.z).length();
    }

    template<class T>
    T TVector3D<T>::distanceFromSQ(const TVector3D<T>& other) const
    {
        return TVector3D<T>(x - other.x, y - other.y, z - other.z).lengthSQ();
    }

    template<class T>
    TVector3D<T>& TVector3D<T>::normalize()
    {
        T l = x * x + y * y + z * z;
        if (l == (T)0.0)
        {
            return *this;
        }
        l = ((T) 1.0) / (T)sqrt((T)l);
        x *= l;
        y *= l;
        z *= l;
        return *this;
    }

    template<class T>
    TVector3D<T>& TVector3D<T>::invert()
    {
        x *= -1.0f;
        y *= -1.0f;
        z *= -1.0f;
        return *this;
    }

    template<class T>
    TVector3D<T>& TVector3D<T>::setLength(T newlength)
    {
        normalize();
        return (*this *= newlength);
    }

} //namespace core
} //namespace v3d
