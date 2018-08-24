namespace v3d
{
namespace core
{

    template<class T>
    TVector4D<T>::TVector4D()
        : x(0)
        , y(0)
        , z(0)
        , w(0)
    {}

    template<class T>
    TVector4D<T>::TVector4D(T nx)
        : x(nx)
        , y(nx)
        , z(nx)
        , w(nx)
    {}

    template<class T>
    TVector4D<T>::TVector4D(T nx, T nw)
        : x(nx)
        , y(nx)
        , z(nx)
        , w(nw)
    {}

    template<class T>
    TVector4D<T>::TVector4D(T nx, T ny, T nz, T nw)
        : x(nx)
        , y(ny)
        , z(nz)
        , w(nw)
    {}

    template<class T>
    TVector4D<T>::TVector4D(const TVector4D<T>& other)
        : x(other.x)
        , y(other.y)
        , z(other.z)
        , w(other.w)
    {}

    template<class T>
    TVector4D<T>::TVector4D(const TVector3D<T>& other, T nw)
        : x(other.x)
        , y(other.y)
        , z(other.z)
        , w(nw)
    {}

    template<class T>
    TVector4D<T> TVector4D<T>::operator - () const
    {
        return TVector4D<T>(-x, -y, -z, -w);
    }

    template<class T>
    TVector4D<T>& TVector4D<T>::operator = (const TVector4D<T>& other)
    {
        x = other.x;
        y = other.y;
        z = other.z;
        w = other.w;
        return *this;
    }

    template<class T>
    TVector4D<T> TVector4D<T>::operator + (const TVector4D<T>& other) const
    {
        return TVector4D<T>(x + other.x, y + other.y, z + other.z, w + other.w);
    }

    template<class T>
    TVector4D<T>& TVector4D<T>::operator += (const TVector4D<T>& other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
        w += other.w;
        return *this;
    }

    template<class T>
    TVector4D<T> TVector4D<T>::operator + (const T scalar) const
    {
        return TVector4D<T>(x + scalar, y + scalar, z + scalar, w + scalar);
    }

    template<class T>
    TVector4D<T>& TVector4D<T>::operator += (const T scalar)
    {
        x += scalar;
        y += scalar;
        z += scalar;
        w += scalar;
        return *this;
    }

    template<class T>
    TVector4D<T> TVector4D<T>::operator - (const TVector4D<T>& other) const
    {
        return TVector4D<T>(x - other.x, y - other.y, z - other.z, w - other.w);
    }

    template<class T>
    TVector4D<T>& TVector4D<T>::operator -= (const TVector4D<T>& other)
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        w -= other.w;
        return *this;
    }

    template<class T>
    TVector4D<T> TVector4D<T>::operator - (const T scalar) const
    {
        return TVector4D<T>(x - scalar, y - scalar, z - scalar, w - scalar);
    }

    template<class T>
    TVector4D<T>& TVector4D<T>::operator -= (const T scalar)
    {
        x -= scalar;
        y -= scalar;
        z -= scalar;
        w -= scalar;
        return *this;
    }

    template<class T>
    TVector4D<T> TVector4D<T>::operator * (const TVector4D<T>& other) const
    {
        return TVector4D<T>(x * other.x, y * other.y, z * other.z, w * other.w);
    }

    template<class T>
    TVector4D<T>& TVector4D<T>::operator *= (const TVector4D<T>& other)
    {
        x *= other.x;
        y *= other.y;
        z *= other.z;
        w *= other.w;
        return *this;
    }

    template<class T>
    TVector4D<T> TVector4D<T>::operator * (const T scalar) const
    {
        return TVector4D<T>(x * scalar, y * scalar, z * scalar, w * scalar);
    }

    template<class T>
    TVector4D<T>& TVector4D<T>::operator *= (const T scalar)
    {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        w *= scalar;
        return *this;
    }

    template<class T>
    TVector4D<T> TVector4D<T>::operator / (const TVector4D<T>& other) const
    {
        return TVector4D<T>(x / other.x, y / other.y, z / other.z, w / other.w);
    }

    template<class T>
    TVector4D<T>& TVector4D<T>::operator /= (const TVector4D<T>& other)
    {
        x /= other.x;
        y /= other.y;
        z /= other.z;
        w /= other.w;
        return *this;
    }

    template<class T>
    TVector4D<T> TVector4D<T>::operator / (const T scalar) const
    {
        T i = (T)1.0 / scalar;
        return TVector4D<T>(x * i, y * i, z * i, w * i);
    }

    template<class T>
    TVector4D<T>& TVector4D<T>::operator /= (const T scalar)
    {
        T i = (T)1.0 / scalar;
        x *= i;
        y *= i;
        z *= i;
        w *= i;
        return *this;
    }

    template<class T>
    bool TVector4D<T>::operator <= (const TVector4D<T>&other) const
    {
        return x <= other.x && y <= other.y && z <= other.z && w <= other.w;
    }

    template<class T>
    bool TVector4D<T>::operator >= (const TVector4D<T>&other) const
    {
        return x >= other.x && y >= other.y && z >= other.z && w >= other.w;
    }

    template<class T>
    bool TVector4D<T>::operator < (const TVector4D<T>&other) const
    {
        return x < other.x && y < other.y && z < other.z && w < other.w;
    }

    template<class T>
    bool TVector4D<T>::operator >(const TVector4D<T>&other) const
    {
        return x > other.x && y > other.y && z > other.z && w > other.w;
    }

    template<class T>
    bool TVector4D<T>::operator == (const TVector4D<T>& other) const
    {
        return this->isEquals(other);
    }

    template<class T>
    bool TVector4D<T>::operator != (const TVector4D<T>& other) const
    {
        return !this->isEquals(other);
    }

    template<class T>
    T TVector4D<T>::operator [] (u32 index) const
    {
        return ((T*)this)[index];
    }

    template<class T>
    T& TVector4D<T>::operator [] (u32 index)
    {
        return ((T*)this)[index];
    }

    template<class T>
    bool TVector4D<T>::isEquals(const TVector4D<T>& other, const T tolerance) const
    {
        const bool isEquals = core::isEquals(x, other.x, tolerance) &&
            core::isEquals(y, other.y, tolerance) &&
            core::isEquals(z, other.z, tolerance) &&
            core::isEquals(w, other.w, tolerance);
        return isEquals;
    }

    template<class T>
    TVector4D<T>& TVector4D<T>::set(const T nx, const T ny, const T nz, const T nw)
    {
        x = nx;
        y = ny;
        z = nz;
        w = nw;
        return *this;
    }

    template<class T>
    TVector4D<T>& TVector4D<T>::set(const TVector4D<T>& point)
    {
        x = point.x;
        y = point.y;
        z = point.z;
        w = point.w;
        return *this;
    }

    template<class T>
    TVector4D<T>& TVector4D<T>::set(const TVector3D<T>& point, const T nw)
    {
        x = point.x;
        y = point.y;
        z = point.z;
        w = nw;
        return *this;
    }

    template<class T>
    T TVector4D<T>::length() const
    {
        const T length = (T)sqrt((f64)(x * x + y * y + z * z + w * w));
        return length;
    }

    template<class T>
    T TVector4D<T>::lengthSQ() const
    {
        return x * x + y * y + z * z + w * w;
    }

    template<class T>
    T TVector4D<T>::distanceFrom(const TVector4D<T>& other) const
    {
        const T distance = TVector4D<T>(x - other.x, y - other.y, z - other.z, w - other.w).length();
        return distance;
    }

    template<class T>
    T TVector4D<T>::distanceFromSQ(const TVector4D<T>& other) const
    {
        const T distanceSQ = TVector4D<T>(x - other.x, y - other.y, z - other.z, w - other.w).lengthSQ();
        return distanceSQ;
    }

    template<class T>
    bool TVector4D<T>::isInLine(const TVector4D<T>& begin, const TVector4D<T>& end) const
    {
        const T f = (end - begin).lengthSQ();
        const bool isInLine = distanceFromSQ(begin) <= f && distanceFromSQ(end) <= f;
        return isInLine;
    }

    template<class T>
    TVector4D<T>& TVector4D<T>::normalize()
    {
        T l = x * x + y * y + z * z + w * w;
        if (l == (T)0.0)
        {
            return *this;
        }
        l = ((T) 1.0) / (T)sqrt((T)l);
        x *= l;
        y *= l;
        z *= l;
        w *= l;
        return *this;
    }

    template<class T>
    TVector4D<T>& TVector4D<T>::setLength(T newlength)
    {
        normalize();
        return (*this *= newlength);
    }

    template<class T>
    TVector4D<T>& TVector4D<T>::invert()
    {
        x *= -1.0f;
        y *= -1.0f;
        z *= -1.0f;
        w *= -1.0f;
        return *this;
    }

} //namespace core
} //namespace v3d
