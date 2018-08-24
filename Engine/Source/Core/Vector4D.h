#pragma once

#include "Math.h"
#include "Vector3D.h"

namespace v3d
{
namespace core
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template <class T>
    class TVector4D
    {
    public:

        TVector4D();
        TVector4D(T nx);
        TVector4D(T nx, T nw);
        TVector4D(T nx, T ny, T nz, T nw = 0);
        TVector4D(const TVector4D<T>& other);
        TVector4D(const TVector3D<T>& other, T nw = 0);

        TVector4D<T>    operator - ()                               const;
        TVector4D<T>&   operator =  (const TVector4D<T>& other);
        TVector4D<T>    operator +  (const TVector4D<T>& other)     const;
        TVector4D<T>&   operator += (const TVector4D<T>& other);
        TVector4D<T>    operator +  (const T scalar)                const;
        TVector4D<T>&   operator += (const T scalar);
        TVector4D<T>    operator -  (const TVector4D<T>& other)     const;
        TVector4D<T>&   operator -= (const TVector4D<T>& other);
        TVector4D<T>    operator -  (const T scalar)                const;
        TVector4D<T>&   operator -= (const T scalar);
        TVector4D<T>    operator *  (const TVector4D<T>& other)     const;
        TVector4D<T>&   operator *= (const TVector4D<T>& other);
        TVector4D<T>    operator *  (const T scalar)                const;
        TVector4D<T>&   operator *= (const T scalar);
        TVector4D<T>    operator /  (const TVector4D<T>& other)     const;
        TVector4D<T>&   operator /= (const TVector4D<T>& other);
        TVector4D<T>    operator /  (const T scalar)                const;
        TVector4D<T>&   operator /= (const T scalar);
        bool            operator <= (const TVector4D<T>& other)     const;
        bool            operator >= (const TVector4D<T>& other)     const;
        bool            operator <  (const TVector4D<T>& other)     const;
        bool            operator >  (const TVector4D<T>& other)     const;
        bool            operator == (const TVector4D<T>& other)     const;
        bool            operator != (const TVector4D<T>& other)     const;
        T               operator [] (u32 index)                     const;
        T&              operator [] (u32 index);

        TVector4D<T>&   set(const T nx, const T ny, const T nz, const T nw = 0);
        TVector4D<T>&   set(const TVector4D<T>& point);
        TVector4D<T>&   set(const TVector3D<T>& point, const T nw = 0);

        bool            isEquals(const TVector4D<T>& other, const T tolerance = (T)k_tolerance32) const;
        bool            isInLine(const TVector4D<T>& begin, const TVector4D<T>& end) const;

        T               length()                                    const;
        T               lengthSQ()                                  const;

        T               distanceFrom(const TVector4D<T>& other)     const;
        T               distanceFromSQ(const TVector4D<T>& other)   const;

        TVector4D<T>&   normalize();
        TVector4D<T>&   invert();

        TVector4D<T>&   setLength(T newlength);

        T x;
        T y;
        T z;
        T w;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    using Point4D    = TVector4D<s32>;
    using Point4D64  = TVector4D<s64>;

    using Point4DU   = TVector4D<u32>;
    using Point4DU64 = TVector4D<u64>;

    using Vector4D   = TVector4D<f32>;
    using Vector4D64 = TVector4D<f64>;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<class T>
    T distance(const TVector4D<T>& v1, const TVector4D<T>& v2)
    {
        const T distance = TVector4D<T>(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w).getLength();
        return distance;
    }

    template<class T>
    T dotProduct(const TVector4D<T>& v1, const TVector4D<T>& v2)
    {
        return v1.x * v2.x + v1.y * v2.y + v2.z * v2.z + v2.w * v2.w;
    }

    template<class T>
    TVector4D<T> crossProduct(const TVector4D<T>& v1, const TVector4D<T>& v2)
    {
        return TVector4D<T>(v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x);
    }

    template<class T>
    TVector4D<T> interpolate(const TVector4D<T>& v1, const TVector4D<T>& v2, const T d)
    {
        return v1 + (v2 - v1) * d;
    }

    template<class S, class T>
    TVector4D<T> operator * (const S scalar, const TVector4D<T>& vector)
    {
        return vector * scalar;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace core
} //namespace v3d

#include "Vector4D.inl"
