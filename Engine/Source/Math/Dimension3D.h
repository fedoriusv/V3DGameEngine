#pragma once

#include "Math.h"

namespace v3d
{
namespace math
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief TDimension3D class
    */
    template <class T>
    class TDimension3D
    {
    public:

        TDimension3D() noexcept;
        TDimension3D(const T& width, const T& height, const T& depth) noexcept;
        TDimension3D(const TDimension3D<T>& other) noexcept;
        ~TDimension3D() = default;

        TDimension3D<T>& operator=(const TDimension3D<T>& other);

        [[nodiscard]] TDimension3D<T> operator+(const TDimension3D<T>& other) const;
        TDimension3D<T>& operator+=(const TDimension3D<T>& other);
        [[nodiscard]] TDimension3D<T> operator-(const TDimension3D<T>& other) const;
        TDimension3D<T>& operator-=(const TDimension3D<T>& other);
        [[nodiscard]] TDimension3D<T> operator*(const T& scale) const;
        TDimension3D<T>& operator*=(const T& scale);
        [[nodiscard]] TDimension3D<T> operator/(const T& scale) const;
        TDimension3D<T>& operator/=(const T& scale);

        bool operator==(const TDimension3D<T>& other) const;
        bool operator!=(const TDimension3D<T>& other) const;
        bool operator>=(const TDimension3D<T>& other) const;
        bool operator<=(const TDimension3D<T>& other) const;
        bool operator>(const TDimension3D<T>& other) const;
        bool operator<(const TDimension3D<T>& other) const;

        TDimension3D<T>& set(const T& width, const T& height, const T& depth);

        [[nodiscard]] T getArea() const;

        bool isNull() const;

        T m_width;
        T m_height;
        T m_depth;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    using Dimension3D = TDimension3D<u32>;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace math
} //namespace v3d

#include "Dimension3D.inl"
