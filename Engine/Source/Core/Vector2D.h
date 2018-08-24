#pragma once

#include "Math.h"

namespace v3d
{
namespace core
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template <class T>
    class TVector2D
    {
    public:

        TVector2D();
        TVector2D(T nx, T ny);
        TVector2D(T nx);
        TVector2D(const TVector2D<T>& other);

        TVector2D<T>    operator -  () const;
        TVector2D<T>&   operator =  (const TVector2D<T>& other);
        TVector2D<T>&   operator =  (const T* other);
        TVector2D<T>    operator +  (const TVector2D<T>& other)    const;
        TVector2D<T>&   operator += (const TVector2D<T>& other);
        TVector2D<T>    operator +  (const T scalar)               const;
        TVector2D<T>&   operator += (const T scalar);
        TVector2D<T>    operator -  (const TVector2D<T>& other)    const;
        TVector2D<T>&   operator -= (const TVector2D<T>& other);
        TVector2D<T>    operator -  (const T scalar)               const;
        TVector2D<T>&   operator -= (const T scalar);
        TVector2D<T>    operator *  (const TVector2D<T>& other)    const;
        TVector2D<T>&   operator *= (const TVector2D<T>& other);
        TVector2D<T>    operator *  (const T scalar)               const;
        TVector2D<T>&   operator *= (const T scalar);
        TVector2D<T>    operator /  (const TVector2D<T>& other)    const;
        TVector2D<T>&   operator /= (const TVector2D<T>& other);
        TVector2D<T>    operator /  (const T scalar)               const;
        TVector2D<T>&   operator /= (const T scalar);
        bool            operator <= (const TVector2D<T>&other)     const;
        bool            operator >= (const TVector2D<T>&other)     const;
        bool            operator <  (const TVector2D<T>&other)     const;
        bool            operator >  (const TVector2D<T>&other)     const;
        bool            operator == (const TVector2D<T>& other)    const;
        bool            operator != (const TVector2D<T>& other)    const;
        T               operator [] (u32 index)                    const;
        T&              operator [] (u32 index);

        TVector2D<T>&   set(const T nx, const T ny);
        TVector2D<T>&   set(const TVector2D<T>& other);

        bool            isEquals(const TVector2D<T>& other, const T tolerance = (T)k_tolerance32)  const;
        bool            isInLine(const TVector2D<T>& begin, const TVector2D<T>& end)               const;

        T               length()                                  const;
        T               lengthSQ()                                const;

        T               distanceFrom(const TVector2D<T>& other)   const;
        T               distanceFromSQ(const TVector2D<T>& other) const;

        TVector2D<T>&   normalize();
        TVector2D<T>&   invert();

        TVector2D<T>&   setLength(T newlength);

        T x;
        T y;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    using Vector2D      = TVector2D<f32>;
    using Vector2D64    = TVector2D<f64>;

    using Point2D       = TVector2D<s32>;
    using Point2D64     = TVector2D<s64>;

    using Point2DU      = TVector2D<u32>;
    using Point2DU64    = TVector2D<u64>;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<class T>
    T distance(const TVector2D<T>& v1, const TVector2D<T>& v2)
    {
        return TVector2D<T>(v1.x - v2.x, v1.y - v2.y).length();
    }

    template<class T>
    T dotProduct(const TVector2D<T>& v1, const TVector2D<T>& v2)
    {
        return v1.x * v2.x + v1.y * v2.y;
    }

    template<class T>
    TVector2D<T> crossProduct(const TVector2D<T>& v1, const TVector2D<T>& v2)
    {
        return (v1.x * v2.y) - (v1.y * v2.x);
    }

    template<class T>
    TVector2D<T> interpolate(const TVector2D<T>& v1, const TVector2D<T>& v2, const T d)
    {
        return v1 + (v2 - v1) * d;
    }

    template<class S, class T>
    TVector2D<T> operator * (const S scalar, const TVector2D<T>& vector)
    {
        return vector * scalar;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace core
} //namespace v3d

#include "Vector2D.inl"
