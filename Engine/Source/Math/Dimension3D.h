#pragma once

#include "Math.h"
#include "Dimension2D.h"

namespace v3d
{
namespace math
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<typename T>
    concept DimensionType3D = std::is_unsigned_v<T>;

    /**
    * @brief TDimension3D struct
    */
    template <DimensionType3D T>
    struct TDimension3D
    {
    public:

        constexpr TDimension3D() noexcept;
        constexpr TDimension3D(T width, T height, T depth) noexcept;
        constexpr TDimension3D(const TDimension3D<T>& other) noexcept;
        constexpr TDimension3D(const TDimension2D<T>& other, T depth = 1U) noexcept;
        ~TDimension3D() = default;

        TDimension3D<T>& operator=(const TDimension3D<T>& other);

        [[nodiscard]] TDimension3D<T> operator+(const TDimension3D<T>& other) const;
        TDimension3D<T>& operator+=(const TDimension3D<T>& other);
        [[nodiscard]] TDimension3D<T> operator-(const TDimension3D<T>& other) const;
        TDimension3D<T>& operator-=(const TDimension3D<T>& other);
        [[nodiscard]] TDimension3D<T> operator*(T scale) const;
        TDimension3D<T>& operator*=(T scale);
        [[nodiscard]] TDimension3D<T> operator/(T scale) const;
        TDimension3D<T>& operator/=(T scale);

        bool operator==(const TDimension3D<T>& other) const;
        bool operator!=(const TDimension3D<T>& other) const;
        bool operator>=(const TDimension3D<T>& other) const;
        bool operator<=(const TDimension3D<T>& other) const;
        bool operator>(const TDimension3D<T>& other) const;
        bool operator<(const TDimension3D<T>& other) const;

        operator TDimension2D<T>() const;

        TDimension3D<T>& set(T width, T height, T depth);

        [[nodiscard]] T getArea() const;

        [[nodiscard]] bool isNull() const;
        [[nodiscard]] bool isValid() const;


        T _width;
        T _height;
        T _depth;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    using Dimension3D = TDimension3D<u32>;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace math
} //namespace v3d

#include "Dimension3D.inl"
