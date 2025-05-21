#pragma once

#include "Math.h"

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

        TDimension3D() noexcept;
        TDimension3D(T width, T height, T depth) noexcept;
        TDimension3D(const TDimension3D<T>& other) noexcept;
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
