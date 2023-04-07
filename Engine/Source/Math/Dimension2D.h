#pragma once

#include "Math.h"

namespace v3d
{
namespace math
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief TDimension2D class
    */
    template <class T>
    class TDimension2D
    {
    public:

        TDimension2D() noexcept;
        TDimension2D(const T& width, const T& height) noexcept;
        TDimension2D(const TDimension2D<T>& other) noexcept;
        ~TDimension2D() = default;

        TDimension2D<T>& operator=(const TDimension2D<T>& other);

        [[nodiscard]] TDimension2D<T> operator+(const TDimension2D<T>& other) const;
        TDimension2D<T>& operator+=(const TDimension2D<T>& other);
        [[nodiscard]] TDimension2D<T> operator-(const TDimension2D<T>& other) const;
        TDimension2D<T>& operator-=(const TDimension2D<T>& other);
        [[nodiscard]] TDimension2D<T> operator*(const T& scale) const;
        TDimension2D<T>& operator*=(const T& scale);
        [[nodiscard]] TDimension2D<T> operator/(const T& scale) const;
        TDimension2D<T>& operator/=(const T& scale);

        bool operator==(const TDimension2D<T>& other) const;
        bool operator!=(const TDimension2D<T>& other) const;

        TDimension2D<T>& set(const T& width, const T& height);

        [[nodiscard]] T getArea() const;

        bool isNull() const;

        T m_width;
        T m_height;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    using Dimension2D = TDimension2D<u32>;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace math
} //namespace v3d

#include "Dimension2D.inl"
