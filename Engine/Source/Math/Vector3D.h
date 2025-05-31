#pragma once

#include "Math.h"
#include "Vector2D.h"

namespace v3d
{
namespace math
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief TVector3D class
    */
    template <class T>
    class TVector3D
    {
    public:

        TVector3D() noexcept;
        TVector3D(T n) noexcept;
        TVector3D(T x, T y, T z) noexcept;
        TVector3D(const TVector3D<T>& other) noexcept;
        TVector3D(const TVector2D<T>& other, T z) noexcept;
        ~TVector3D() noexcept = default;

        [[nodiscard]] T operator[](u32 index) const;
        T& operator[](u32 index);

        TVector3D<T>& operator=(const TVector3D<T>& other);

        [[nodiscard]] TVector3D<T> operator+(const TVector3D<T>& other) const;
        [[nodiscard]] TVector3D<T> operator+(T scalar) const;
        TVector3D<T>& operator+=(const TVector3D<T>& other);
        TVector3D<T>& operator+=(T scalar);
        [[nodiscard]] TVector3D<T> operator-(const TVector3D<T>& other) const;
        [[nodiscard]] TVector3D<T> operator-(T scalar) const;
        TVector3D<T>& operator-=(const TVector3D<T>& other);
        TVector3D<T>& operator-=(T scalar);
        [[nodiscard]] TVector3D<T> operator*(const TVector3D<T>& other) const;
        [[nodiscard]] TVector3D<T> operator*(T scalar) const;
        TVector3D<T>& operator*=(const TVector3D<T>& other);
        TVector3D<T>& operator*=(T scalar);
        [[nodiscard]] TVector3D<T> operator/(const TVector3D<T>& other) const;
        [[nodiscard]] TVector3D<T> operator/(T scalar) const;
        TVector3D<T>& operator/=(const TVector3D<T>& other);
        TVector3D<T>& operator/=(T scalar);

        [[nodiscard]] TVector3D<T> operator-() const;

        bool operator<=(const TVector3D<T>&other) const;
        bool operator>=(const TVector3D<T>&other) const;
        bool operator<(const TVector3D<T>&other) const;
        bool operator>(const TVector3D<T>&other) const;
        bool operator==(const TVector3D<T>& other) const;
        bool operator!=(const TVector3D<T>& other) const;

        TVector3D<T>& set(T x, T y, T z);
        TVector3D<T>& set(const TVector3D<T>& p);

        [[nodiscard]] bool isEquals(const TVector3D<T>& other, T tolerance = (T)k_tolerance32) const;
        [[nodiscard]] bool isInLine(const TVector3D<T>& begin, const TVector3D<T>& end) const;

        [[nodiscard]] T length() const;
        [[nodiscard]] T lengthSQ() const;

        [[nodiscard]] T distanceFrom(const TVector3D<T>& other) const;
        [[nodiscard]] T distanceFromSQ(const TVector3D<T>& other) const;

        TVector3D<T>& normalize();

        T _x;
        T _y;
        T _z;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    using float3 = TVector3D<f32>;
    using int3 = TVector3D<s32>;

    using Point3D    = TVector3D<s32>;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace math
} //namespace v3d

#include "Vector3D.inl"
