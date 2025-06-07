#pragma once

#include "Math.h"

namespace v3d
{
namespace math
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief TVector2D struct
    */
    template <typename T>
    struct TVector2D
    {
    public:

        constexpr TVector2D() noexcept;
        constexpr TVector2D(T n) noexcept;
        constexpr TVector2D(T x, T y) noexcept;
        constexpr TVector2D(const TVector2D<T>& other) noexcept;
        ~TVector2D() noexcept = default;

        [[nodiscard]] T operator[](u32 index) const;
        T& operator[](u32 index);

        TVector2D<T>& operator=(const TVector2D<T>& other);

        [[nodiscard]] TVector2D<T> operator+(const TVector2D<T>& other) const;
        [[nodiscard]] TVector2D<T> operator+(T scalar) const;
        TVector2D<T>& operator+=(const TVector2D<T>& other);
        TVector2D<T>& operator+=(T scalar);
        [[nodiscard]] TVector2D<T> operator-(const TVector2D<T>& other) const;
        [[nodiscard]] TVector2D<T> operator-(T scalar) const;
        TVector2D<T>& operator-=(const TVector2D<T>& other);
        TVector2D<T>& operator-=(T scalar);
        [[nodiscard]] TVector2D<T> operator*(const TVector2D<T>& other) const;
        [[nodiscard]] TVector2D<T> operator*(T scalar) const;
        TVector2D<T>& operator*=(const TVector2D<T>& other);
        TVector2D<T>& operator*=(T scalar);
        [[nodiscard]] TVector2D<T> operator/(const TVector2D<T>& other) const;
        [[nodiscard]] TVector2D<T> operator/(T scalar) const;
        TVector2D<T>& operator/=(const TVector2D<T>& other);
        TVector2D<T>& operator/=(T scalar);

        [[nodiscard]] TVector2D<T> operator-() const;

        bool operator<=(const TVector2D<T>&other) const;
        bool operator>=(const TVector2D<T>&other) const;
        bool operator<(const TVector2D<T>&other) const;
        bool operator>(const TVector2D<T>&other) const;
        bool operator==(const TVector2D<T>& other) const;
        bool operator!=(const TVector2D<T>& other) const;

        TVector2D<T>& set(T x, T y);
        TVector2D<T>& set(const TVector2D<T>& other);

        [[nodiscard]] bool isEquals(const TVector2D<T>& other, T tolerance = (T)k_tolerance32) const;

        [[nodiscard]] T length() const;
        [[nodiscard]] T lengthSQ() const;

        [[nodiscard]] T distanceFrom(const TVector2D<T>& other) const;
        [[nodiscard]] T distanceFromSQ(const TVector2D<T>& other) const;

        TVector2D<T>& normalize();

        T _x;
        T _y;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    using float2    = TVector2D<f32>;
    using int2      = TVector2D<s32>;

    using Point2D     = TVector2D<s32>;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace math
} //namespace v3d

#include "Vector2D.inl"
