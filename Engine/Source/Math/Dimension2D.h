#pragma once

#include "Math.h"

namespace v3d
{
namespace math
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<typename T>
    concept DimensionType2D = std::is_unsigned_v<T>;

    /**
    * @brief TDimension2D struct
    */
    template <DimensionType2D T>
    struct TDimension2D
    {
    public:

        constexpr TDimension2D() noexcept;
        constexpr TDimension2D(T width, T height) noexcept;
        constexpr TDimension2D(const TDimension2D<T>& other) noexcept;
        ~TDimension2D() = default;

        TDimension2D<T>& operator=(const TDimension2D<T>& other);

        [[nodiscard]] TDimension2D<T> operator+(const TDimension2D<T>& other) const;
        TDimension2D<T>& operator+=(const TDimension2D<T>& other);
        [[nodiscard]] TDimension2D<T> operator-(const TDimension2D<T>& other) const;
        TDimension2D<T>& operator-=(const TDimension2D<T>& other);
        [[nodiscard]] TDimension2D<T> operator*(T scale) const;
        TDimension2D<T>& operator*=(T scale);
        [[nodiscard]] TDimension2D<T> operator/(T scale) const;
        TDimension2D<T>& operator/=(T scale);

        bool operator==(const TDimension2D<T>& other) const;
        bool operator!=(const TDimension2D<T>& other) const;

        TDimension2D<T>& set(T width, T height);

        [[nodiscard]] T getArea() const;

        [[nodiscard]] bool isNull() const;
        [[nodiscard]] bool isValid() const;

        T _width;
        T _height;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    using Dimension2D = TDimension2D<u32>;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace math
} //namespace v3d

#include "Dimension2D.inl"
