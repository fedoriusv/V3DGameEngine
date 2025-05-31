#pragma once

#include "Math.h"
#include "Vector3D.h"

namespace v3d
{
namespace math
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief TVector4D class
    */
    template <class T>
    class TVector4D
    {
    public:

        TVector4D() noexcept;
        TVector4D(T n) noexcept;
        TVector4D(T x, T y, T z, T w = 0) noexcept;
        TVector4D(const TVector4D<T>& other) noexcept;
        TVector4D(const TVector3D<T>& other, T w) noexcept;
        ~TVector4D() noexcept = default;

        [[nodiscard]] T operator[](u32 index) const;
        T& operator[](u32 index);

        TVector4D<T>& operator=(const TVector4D<T>& other);

        [[nodiscard]] TVector4D<T> operator+(const TVector4D<T>& other) const;
        [[nodiscard]] TVector4D<T> operator+(const T scalar) const;
        TVector4D<T>& operator+=(const TVector4D<T>& other);
        TVector4D<T>& operator+=(const T scalar);
        [[nodiscard]] TVector4D<T> operator-(const TVector4D<T>& other) const;
        [[nodiscard]] TVector4D<T> operator-(const T scalar) const;
        TVector4D<T>& operator-=(const TVector4D<T>& other);
        TVector4D<T>& operator-=(const T scalar);
        [[nodiscard]] TVector4D<T> operator*(const TVector4D<T>& other) const;
        [[nodiscard]] TVector4D<T> operator*(const T scalar) const;
        TVector4D<T>& operator*=(const TVector4D<T>& other);
        TVector4D<T>& operator*=(const T scalar);
        [[nodiscard]] TVector4D<T> operator/(const TVector4D<T>& other) const;
        [[nodiscard]] TVector4D<T> operator/(const T scalar) const;
        TVector4D<T>& operator/=(const TVector4D<T>& other);
        TVector4D<T>& operator/=(const T scalar);
        [[nodiscard]] TVector4D<T> operator-() const;

        bool operator<=(const TVector4D<T>& other) const;
        bool operator>=(const TVector4D<T>& other) const;
        bool operator<(const TVector4D<T>& other) const;
        bool operator>(const TVector4D<T>& other) const;
        bool operator==(const TVector4D<T>& other) const;
        bool operator!=(const TVector4D<T>& other) const;

        TVector4D<T>& set(T x, T y, T z, T w = 0);
        TVector4D<T>& set(const TVector4D<T>& point);
        TVector4D<T>& set(const TVector3D<T>& point, T w);

        [[nodiscard]] bool isEquals(const TVector4D<T>& other, T tolerance = (T)k_tolerance32) const;
        [[nodiscard]] bool isInLine(const TVector4D<T>& begin, const TVector4D<T>& end) const;

        [[nodiscard]] T length() const;
        [[nodiscard]] T lengthSQ() const;

        [[nodiscard]] T distanceFrom(const TVector4D<T>& other) const;
        [[nodiscard]] T distanceFromSQ(const TVector4D<T>& other) const;

        TVector4D<T>& normalize();

        T _x;
        T _y;
        T _z;
        T _w;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    using float4 = TVector4D<f32>;
    using int4 = TVector4D<s32>;

    using Color4D    = TVector4D<u8>;
    using Point4D    = TVector4D<s32>;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace math
} //namespace v3d

#include "Vector4D.inl"
