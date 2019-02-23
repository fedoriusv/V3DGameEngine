#pragma once

#include "Math.h"

namespace v3d
{
namespace core
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template <class T>
    class TDimension3D
    {
    public:

        TDimension3D();
        TDimension3D(const T& width, const T& height, const T& depth);
        TDimension3D(const TDimension3D<T>& other);

        bool                operator == (const TDimension3D<T>& other)  const;
        bool                operator != (const TDimension3D<T>& other)  const;
        bool                operator >= (const TDimension3D<T>& other)  const;
        bool                operator <= (const TDimension3D<T>& other)  const;
        bool                operator > (const TDimension3D<T>& other)  const;
        bool                operator < (const TDimension3D<T>& other)  const;

        TDimension3D<T>&    operator = (const TDimension3D<T>& other);
        TDimension3D<T>&    operator /= (const T& scale);
        TDimension3D<T>     operator /  (const T& scale)                const;
        TDimension3D<T>     operator *  (const T& scale)                const;
        TDimension3D<T>     operator +  (const TDimension3D<T>& other)  const;
        TDimension3D<T>     operator -  (const TDimension3D<T>& other)  const;
        TDimension3D<T>&    operator += (const TDimension3D<T>& other);
        TDimension3D<T>&    operator -= (const TDimension3D<T>& other);
        TDimension3D<T>&    operator *= (const T& scale);

        TDimension3D<T>&    set(const T& width, const T& height, const T& depth);

        T                   getArea() const;

        bool                isNull() const;

        T                   width;
        T                   height;
        T                   depth;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    using Dimension3D = TDimension3D<u32>;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace core
} //namespace v3d

#include "Dimension3D.inl"
