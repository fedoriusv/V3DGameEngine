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
        TVector3D(T nx) noexcept;
        TVector3D(T nx, T ny, T nz) noexcept;
        TVector3D(const TVector3D<T>& other) noexcept;
        TVector3D(const TVector2D<T>& other, T nz = 0) noexcept;
        ~TVector3D() noexcept = default;

        [[nodiscard]] T operator[](u32 index) const;
        T& operator[](u32 index);

        TVector3D<T>& operator=(const TVector3D<T>& other);
        TVector3D<T>& operator=(const T* other);

        [[nodiscard]] TVector3D<T> operator+(const TVector3D<T>& other) const;
        [[nodiscard]] TVector3D<T> operator+(const T scalar) const;
        TVector3D<T>& operator+=(const TVector3D<T>& other);
        TVector3D<T>& operator+=(const T scalar);
        [[nodiscard]] TVector3D<T> operator-(const TVector3D<T>& other) const;
        [[nodiscard]] TVector3D<T> operator-(const T scalar) const;
        TVector3D<T>& operator-=(const TVector3D<T>& other);
        TVector3D<T>& operator-=(const T scalar);
        [[nodiscard]] TVector3D<T> operator*(const TVector3D<T>& other) const;
        [[nodiscard]] TVector3D<T> operator*(const T scalar) const;
        TVector3D<T>& operator*=(const TVector3D<T>& other);
        TVector3D<T>& operator*=(const T scalar);
        [[nodiscard]] TVector3D<T> operator/(const TVector3D<T>& other) const;
        [[nodiscard]] TVector3D<T> operator/(const T scalar) const;
        TVector3D<T>& operator/=(const TVector3D<T>& other);
        TVector3D<T>& operator/=(const T scalar);

        [[nodiscard]] TVector3D<T> operator-() const;

        bool operator<=(const TVector3D<T>&other) const;
        bool operator>=(const TVector3D<T>&other) const;
        bool operator<(const TVector3D<T>&other) const;
        bool operator>(const TVector3D<T>&other) const;
        bool operator==(const TVector3D<T>& other) const;
        bool operator!=(const TVector3D<T>& other) const;

        TVector3D<T>& set(const T nx, const T ny, const T nz);
        TVector3D<T>& set(const TVector3D<T>& p);

        [[nodiscard]] bool isEquals(const TVector3D<T>& other, const T tolerance = (T)k_tolerance32) const;
        [[nodiscard]] bool isInLine(const TVector3D<T>& begin, const TVector3D<T>& end) const;

        [[nodiscard]] T length() const;
        [[nodiscard]] T lengthSQ() const;

        [[nodiscard]] T distanceFrom(const TVector3D<T>& other) const;
        [[nodiscard]] T distanceFromSQ(const TVector3D<T>& other) const;

        TVector3D<T>& normalize();
        TVector3D<T>& invert();

        TVector3D<T>& setLength(T newlength);

        T m_x;
        T m_y;
        T m_z;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    using Vector3D   = TVector3D<f32>;
    using Vector3D64 = TVector3D<f64>;

    using Point3D    = TVector3D<s32>;
    using Point3D64  = TVector3D<s64>;

    using Point3DU   = TVector3D<u32>;
    using Point3DU64 = TVector3D<u64>;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<class T>
    [[nodiscard]] T distance(const TVector3D<T>& v1, const TVector3D<T>& v2)
    {
        const T distance = TVector3D<T>(v1.m_x - v2.m_x, v1.m_y - v2.m_y, v1.m_z - v2.m_z).length();
        return distance;
    }

    template<class T>
    [[nodiscard]] T dotProduct(const TVector3D<T>& v1, const TVector3D<T>& v2)
    {
        return v1.m_x * v2.m_x + v1.m_y * v2.m_y + v1.m_z * v2.m_z;
    }

    template<class T>
    [[nodiscard]] TVector3D<T> crossProduct(const TVector3D<T>& v1, const TVector3D<T>& v2)
    {
        return TVector3D<T>(v1.m_y * v2.m_z - v1.m_z * v2.m_y, v1.m_z * v2.m_x - v1.m_x * v2.m_z, v1.m_x * v2.m_y - v1.m_y * v2.m_x);
    }

    template<class T>
    [[nodiscard]] TVector3D<T> interpolate(const TVector3D<T>& v1, const TVector3D<T>& v2, const T d)
    {
        return v1 + (v2 - v1) * d;
    }

    template<class S, class T>
    [[nodiscard]] TVector3D<T> operator*(const S scalar, const TVector3D<T>& vector)
    {
        return vector * scalar;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace math
} //namespace v3d

#include "Vector3D.inl"
