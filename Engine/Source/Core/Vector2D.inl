namespace v3d
{
namespace core
{

    template<class T>
    TVector2D<T>::TVector2D()
        : x(0)
        , y(0)
    {}

    template<class T>
    TVector2D<T>::TVector2D(T nx, T ny)
        : x(nx)
        , y(ny)
    {}

    template<class T>
    TVector2D<T>::TVector2D(T nx)
        : x(nx)
        , y(nx)
    {}

    template<class T>
    TVector2D<T>::TVector2D(const TVector2D<T>& other)
        : x(other.x)
        , y(other.y)
    {}

    template<class T>
    TVector2D<T> TVector2D<T>::operator - () const
    {
        return TVector2D<T>(-x, -y);
    }

    template<class T>
    TVector2D<T>& TVector2D<T>::operator = (const TVector2D<T>& other)
    {
        if (this == &other)
        {
            return *this;
        }

        x = other.x;
        y = other.y;

        return *this;
    }

    template<class T>
    TVector2D<T>&	TVector2D<T>::operator =  (const T* other)
    {
        x = other[0];
        y = other[1];

        return *this;
    }

    template<class T>
    TVector2D<T> TVector2D<T>::operator + (const TVector2D<T>& other) const
    {
        return TVector2D<T>(x + other.x, y + other.y);
    }

    template<class T>
    TVector2D<T>& TVector2D<T>::operator += (const TVector2D<T>& other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    template<class T>
    TVector2D<T> TVector2D<T>::operator + (const T scalar) const
    {
        return TVector2D<T>(x + scalar, y + scalar);
    }

    template<class T>
    TVector2D<T>& TVector2D<T>::operator += (const T scalar)
    {
        x += scalar;
        y += scalar;
        return *this;
    }

    template<class T>
    TVector2D<T> TVector2D<T>::operator - (const TVector2D<T>& other) const
    {
        return TVector2D<T>(x - other.x, y - other.y);
    }

    template<class T>
    TVector2D<T>& TVector2D<T>::operator -= (const TVector2D<T>& other)
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    template<class T>
    TVector2D<T> TVector2D<T>::operator - (const T scalar) const
    {
        return TVector2D<T>(x - scalar, y - scalar);
    }

    template<class T>
    TVector2D<T>& TVector2D<T>::operator -= (const T scalar)
    {
        x -= scalar;
        y -= scalar;
        return *this;
    }

    template<class T>
    TVector2D<T> TVector2D<T>::operator * (const TVector2D<T>& other) const
    {
        return TVector2D<T>(x * other.x, y * other.y);
    }

    template<class T>
    TVector2D<T>& TVector2D<T>::operator *= (const TVector2D<T>& other)
    {
        x *= other.x;
        y *= other.y;
        return *this;
    }

    template<class T>
    TVector2D<T> TVector2D<T>::operator * (const T scalar) const
    {
        return TVector2D<T>(x * scalar, y * scalar);
    }

    template<class T>
    TVector2D<T>& TVector2D<T>::operator *= (const T scalar)
    {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    template<class T>
    TVector2D<T> TVector2D<T>::operator / (const TVector2D<T>& other) const
    {
        return TVector2D<T>(x / other.x, y / other.y);
    }

    template<class T>
    TVector2D<T>& TVector2D<T>::operator /= (const TVector2D<T>& other)
    {
        x /= other.x;
        y /= other.y;
        return *this;
    }

    template<class T>
    TVector2D<T> TVector2D<T>::operator / (const T v) const
    {
        T i = (T)1.0 / v;
        TVector2D<T> out(x * i, y * i);
        return out;
    }

    template<class T>
    TVector2D<T>& TVector2D<T>::operator /= (const T v)
    {
        T i = (T)1.0 / v;
        x *= i;
        y *= i;
        return *this;
    }

    template<class T>
    bool TVector2D<T>::operator <= (const TVector2D<T>&other) const
    {
        return x <= other.x && y <= other.y;
    }

    template<class T>
    bool TVector2D<T>::operator >= (const TVector2D<T>&other) const
    {
        return x >= other.x && y >= other.y;
    }

    template<class T>
    bool TVector2D<T>::operator < (const TVector2D<T>&other) const
    {
        return x < other.x && y < other.y;
    }

    template<class T>
    bool TVector2D<T>::operator >(const TVector2D<T>&other) const
    {
        return x > other.x && y > other.y;
    }

    template<class T>
    bool TVector2D<T>::operator == (const TVector2D<T>& other) const
    {
        return this->isEquals(other);;
    }

    template<class T>
    bool TVector2D<T>::operator != (const TVector2D<T>& other) const
    {
        return !this->isEquals(other);;
    }

    template<class T>
    T TVector2D<T>::operator [] (u32 index) const
    {
        return ((T*)this)[index];
    }

    template<class T>
    T& TVector2D<T>::operator [] (u32 index)
    {
        return ((T*)this)[index];
    }

    template<class T>
    TVector2D<T>& TVector2D<T>::set(const T nx, const T ny)
    {
        x = nx;
        y = ny;
        return *this;
    }

    template<class T>
    TVector2D<T>& TVector2D<T>::set(const TVector2D<T>& other)
    {
        x = other.x;
        y = other.y;
        return *this;
    }

    template<class T>
    T TVector2D<T>::length() const
    {
        const T length = (T)sqrt((f64)(x * x + y * y));
        return length;
    }

    template<class T>
    T TVector2D<T>::lengthSQ() const
    {
        return x * x + y * y;
    }

    template<class T>
    T TVector2D<T>::distanceFrom(const TVector2D<T>& other) const
    {
        const T distance = TVector2D<T>(x - other.x, y - other.y).length();
        return distance;
    }

    template<class T>
    T TVector2D<T>::distanceFromSQ(const TVector2D<T>& other) const
    {
        const T distanceSQ = TVector2D<T>(x - other.x, y - other.y).lengthSQ();
        return distanceSQ;
    }

    template<class T>
    bool TVector2D<T>::isEquals(const TVector2D<T>& other, const T tolerance) const
    {
        const bool isEquels = core::isEquals(x, other.x, tolerance) &&
            core::isEquals(y, other.y, tolerance);
        return isEquels;
    }

    template<class T>
    bool TVector2D<T>::isInLine(const TVector2D<T>& begin, const TVector2D<T>& end) const
    {
        const T f = (end - begin).lengthSQ();
        const bool isInLine = distanceFromSQ(begin) <= f && distanceFromSQ(end) <= f;
        return isInLine;
    }

    template<class T>
    TVector2D<T>& TVector2D<T>::normalize()
    {
        T l = x * x + y * y;

        if (l == (T) 0.0)
        {
            return *this;
        }
        l = ((T) 1.0) / (T)sqrt((T)l);
        x *= l;
        y *= l;

        return *this;
    }

    template<class T>
    TVector2D<T>& TVector2D<T>::invert()
    {
        x *= -1.0f;
        y *= -1.0f;
        return *this;
    }

    template<class T>
    TVector2D<T>& TVector2D<T>::setLength(T newlength)
    {
        normalize();
        return (*this *= newlength);
    }

} //namespace core
} //namespace v3d
