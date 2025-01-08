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
        TVector4D(T nx) noexcept;
        TVector4D(T nx, T nw) noexcept;
        TVector4D(T nx, T ny, T nz, T nw = 0) noexcept;
        TVector4D(const TVector4D<T>& other) noexcept;
        TVector4D(const TVector3D<T>& other, T nw = 0) noexcept;
        ~TVector4D() noexcept = default;

        [[nodiscard]] T operator[](u32 index) const;
        T& operator[](u32 index);

        TVector4D<T>& operator=(const TVector4D<T>& other);
        TVector4D<T>& operator=(const T* other);

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

        TVector4D<T>& set(const T nx, const T ny, const T nz, const T nw = 0);
        TVector4D<T>& set(const TVector4D<T>& point);
        TVector4D<T>& set(const TVector3D<T>& point, const T nw = 0);

        [[nodiscard]] bool isEquals(const TVector4D<T>& other, const T tolerance = (T)k_tolerance32) const;
        [[nodiscard]] bool isInLine(const TVector4D<T>& begin, const TVector4D<T>& end) const;

        [[nodiscard]] T length() const;
        [[nodiscard]] T lengthSQ() const;

        [[nodiscard]] T distanceFrom(const TVector4D<T>& other) const;
        [[nodiscard]] T distanceFromSQ(const TVector4D<T>& other) const;

        TVector4D<T>& normalize();
        TVector4D<T>& invert();

        TVector4D<T>& setLength(T newlength);

        T m_x;
        T m_y;
        T m_z;
        T m_w;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    using Color4D    = TVector4D<u8>;

    using Point4D    = TVector4D<s32>;
    using Point4D64  = TVector4D<s64>;

    using Point4DU   = TVector4D<u32>;
    using Point4DU64 = TVector4D<u64>;

    using Vector4D   = TVector4D<f32>;
    using Vector4D64 = TVector4D<f64>;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<class T>
    [[nodiscard]] T distance(const TVector4D<T>& v1, const TVector4D<T>& v2)
    {
        const T distance = TVector4D<T>(v1.m_x - v2.m_x, v1.m_y - v2.m_y, v1.m_z - v2.m_z, v1.m_w - v2.m_w).getLength();
        return distance;
    }

    template<class T>
    [[nodiscard]] T dotProduct(const TVector4D<T>& v1, const TVector4D<T>& v2)
    {
        return v1.m_x * v2.m_x + v1.m_y * v2.m_y + v2.m_z * v2.m_z + v2.m_w * v2.m_w;
    }

    template<class T>
    [[nodiscard]] TVector4D<T> crossProduct(const TVector4D<T>& v1, const TVector4D<T>& v2)
    {
        return TVector4D<T>(v1.m_y * v2.m_z - v1.m_z * v2.m_y, v1.m_z * v2.m_x - v1.m_x * v2.m_z, v1.m_x * v2.m_y - v1.m_y * v2.m_x);
    }

    template<class T>
    [[nodiscard]] TVector4D<T> interpolate(const TVector4D<T>& v1, const TVector4D<T>& v2, const T d)
    {
        return v1 + (v2 - v1) * d;
    }

    template<class S, class T>
    [[nodiscard]] TVector4D<T> operator * (const S scalar, const TVector4D<T>& vector)
    {
        return vector * scalar;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace math
} //namespace v3d

#include "Vector4D.inl"
