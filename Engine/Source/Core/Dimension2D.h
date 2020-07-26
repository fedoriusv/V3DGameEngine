#pragma once

#include "Math.h"

namespace v3d
{
namespace core
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template <class T>
    class TDimension2D
    {
    public:

        TDimension2D();
        TDimension2D(const T& width, const T& height);
        TDimension2D(const TDimension2D<T>& other);

        bool                operator==(const TDimension2D<T>& other)    const;
        bool                operator!=(const TDimension2D<T>& other)    const;

        TDimension2D<T>&    operator=(const TDimension2D<T>& other);
        TDimension2D<T>&    operator/=(const T& scale);
        TDimension2D<T>     operator/(const T& scale)                   const;
        TDimension2D<T>     operator*(const T& scale)                   const;
        TDimension2D<T>     operator+(const TDimension2D<T>& other)     const;
        TDimension2D<T>&    operator+=(const TDimension2D<T>& other);
        TDimension2D<T>&    operator-=(const TDimension2D<T>& other);
        TDimension2D<T>&    operator*=(const T& scale);

        TDimension2D<T>&    set(const T& width, const T& height);

        T                   getArea() const;

        bool                isNull() const;

        T                   width;
        T                   height;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    using Dimension2D = TDimension2D<u32>;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace core
} //namespace v3d

#include "Dimension2D.inl"
