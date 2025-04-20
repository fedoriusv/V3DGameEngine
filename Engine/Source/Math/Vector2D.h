#pragma once

#include "Math.h"

namespace v3d
{
namespace math
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief TVector2D class
    */
    template <class T>
    class TVector2D
    {
    public:

        TVector2D() noexcept;
        TVector2D(T nx, T ny) noexcept;
        TVector2D(T nx) noexcept;
        TVector2D(const TVector2D<T>& other) noexcept;
        ~TVector2D() noexcept = default;

        [[nodiscard]] T operator[](u32 index) const;
        T& operator[](u32 index);

        TVector2D<T>& operator=(const TVector2D<T>& other);

        [[nodiscard]] TVector2D<T> operator+(const TVector2D<T>& other) const;
        [[nodiscard]] TVector2D<T> operator+(const T scalar) const;
        TVector2D<T>& operator+=(const TVector2D<T>& other);
        TVector2D<T>& operator+=(const T scalar);
        [[nodiscard]] TVector2D<T> operator-(const TVector2D<T>& other) const;
        [[nodiscard]] TVector2D<T> operator-(const T scalar) const;
        TVector2D<T>& operator-=(const TVector2D<T>& other);
        TVector2D<T>& operator-=(const T scalar);
        [[nodiscard]] TVector2D<T> operator*(const TVector2D<T>& other) const;
        [[nodiscard]] TVector2D<T> operator*(const T scalar) const;
        TVector2D<T>& operator*=(const TVector2D<T>& other);
        TVector2D<T>& operator*=(const T scalar);
        [[nodiscard]] TVector2D<T> operator/(const TVector2D<T>& other) const;
        [[nodiscard]] TVector2D<T> operator/(const T scalar) const;
        TVector2D<T>& operator/=(const TVector2D<T>& other);
        TVector2D<T>& operator/=(const T scalar);

        [[nodiscard]] TVector2D<T> operator-() const;

        bool operator<=(const TVector2D<T>&other) const;
        bool operator>=(const TVector2D<T>&other) const;
        bool operator<(const TVector2D<T>&other) const;
        bool operator>(const TVector2D<T>&other) const;
        bool operator==(const TVector2D<T>& other) const;
        bool operator!=(const TVector2D<T>& other) const;

        TVector2D<T>& set(const T nx, const T ny);
        TVector2D<T>& set(const TVector2D<T>& other);

        [[nodiscard]] bool isEquals(const TVector2D<T>& other, const T tolerance = (T)k_tolerance32) const;
        [[nodiscard]] bool isInLine(const TVector2D<T>& begin, const TVector2D<T>& end) const;

        [[nodiscard]] T length() const;
        [[nodiscard]] T lengthSQ() const;

        [[nodiscard]] T distanceFrom(const TVector2D<T>& other) const;
        [[nodiscard]] T distanceFromSQ(const TVector2D<T>& other) const;

        TVector2D<T>& normalize();
        TVector2D<T>& invert();

        TVector2D<T>& setLength(T newlength);

        T m_x;
        T m_y;
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
    [[nodiscard]] T distance(const TVector2D<T>& v1, const TVector2D<T>& v2)
    {
        return TVector2D<T>(v1.m_x - v2.m_x, v1.m_y - v2.m_y).length();
    }

    template<class T>
    [[nodiscard]] T dotProduct(const TVector2D<T>& v1, const TVector2D<T>& v2)
    {
        return v1.m_x * v2.m_x + v1.m_y * v2.m_y;
    }

    template<class T>
    [[nodiscard]] TVector2D<T> crossProduct(const TVector2D<T>& v1, const TVector2D<T>& v2)
    {
        return (v1.m_x * v2.m_y) - (v1.m_y * v2.m_x);
    }

    template<class T>
    [[nodiscard]] TVector2D<T> interpolate(const TVector2D<T>& v1, const TVector2D<T>& v2, const T d)
    {
        return v1 + (v2 - v1) * d;
    }

    template<class S, class T>
    [[nodiscard]] TVector2D<T> operator*(const S scalar, const TVector2D<T>& vector)
    {
        return vector * scalar;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace math
} //namespace v3d

#include "Vector2D.inl"
