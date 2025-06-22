#include "Dimension3D.h"

namespace v3d
{
namespace math
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template <DimensionType3D T>
    constexpr inline TDimension3D<T>::TDimension3D() noexcept
        : _width(0)
        , _height(0)
        , _depth(0)
    {
    }

    template <DimensionType3D T>
    constexpr inline TDimension3D<T>::TDimension3D(T width, T height, T depth) noexcept
        : _width(width)
        , _height(height)
        , _depth(depth)
    {
    }

    template <DimensionType3D T>
    constexpr inline TDimension3D<T>::TDimension3D(const TDimension3D<T>& other) noexcept
        : _width(other._width)
        , _height(other._height)
        , _depth(other._depth)
    {
    }

    template<DimensionType3D T>
    inline constexpr TDimension3D<T>::TDimension3D(const TDimension2D<T>& other, T depth) noexcept
        : _width(other._width)
        , _height(other._height)
        , _depth(depth)
    {
    }

    template <DimensionType3D T>
    inline TDimension3D<T>& TDimension3D<T>::operator=(const TDimension3D<T>& other)
    {
        if (this == &other)
        {
            return *this;
        }

        _width = other._width;
        _height = other._height;
        _depth = other._depth;

        return *this;
    }

    template <DimensionType3D T>
    inline bool TDimension3D<T>::operator==(const TDimension3D<T>& other) const
    {
        const bool isEquals = 
            math::isEquals(_width, other._width) &&
            math::isEquals(_height, other._height) &&
            math::isEquals(_depth, other._depth);

        return isEquals;
    }

    template <DimensionType3D T>
    inline bool TDimension3D<T>::operator!=(const TDimension3D<T>& other) const
    {
        return !(*this == other);
    }

    template<DimensionType3D T>
    inline bool TDimension3D<T>::operator>=(const TDimension3D<T>& other) const
    {
        return !(*this <= other);
    }

    template<DimensionType3D T>
    inline bool TDimension3D<T>::operator<=(const TDimension3D<T>& other) const
    {
        return (*this < other) || (*this == other);
    }

    template<DimensionType3D T>
    inline bool TDimension3D<T>::operator>(const TDimension3D<T>& other) const
    {
        return !(*this < other);
    }

    template<DimensionType3D T>
    inline bool TDimension3D<T>::operator<(const TDimension3D<T>& other) const
    {
        return _width < other._width && _height < other._height && _depth < other._depth;
    }

    template <DimensionType3D T>
    inline TDimension3D<T>& TDimension3D<T>::set(T width, T height, T depth)
    {
        _width = width;
        _height = height;
        _depth = depth;

        return *this;
    }

    template <DimensionType3D T>
    inline TDimension3D<T>& TDimension3D<T>::operator/=(T scale)
    {
        _width /= scale;
        _height /= scale;
        _depth /= scale;

        return *this;
    }

    template <DimensionType3D T>
    inline TDimension3D<T> TDimension3D<T>::operator/(T scale) const
    {
        return TDimension3D<T>(_width / scale, _height / scale, _depth / scale);
    }

    template <DimensionType3D T>
    inline TDimension3D<T>& TDimension3D<T>::operator *= (T scale)
    {
        _width *= scale;
        _height *= scale;
        _depth *= scale;

        return *this;
    }

    template <DimensionType3D T>
    inline TDimension3D<T> TDimension3D<T>::operator*(T scale) const
    {
        return TDimension3D<T>(_width * scale, _height * scale, _depth * scale);
    }

    template <DimensionType3D T>
    inline TDimension3D<T>& TDimension3D<T>::operator+=(const TDimension3D<T>& other)
    {
        _width += other._width;
        _height += other._height;
        _depth += other._depth;

        return *this;
    }

    template <DimensionType3D T>
    inline TDimension3D<T>& TDimension3D<T>::operator-=(const TDimension3D<T>& other)
    {
        _width -= other._width;
        _height -= other._height;
        _depth -= other._depth;

        return *this;
    }

    template <DimensionType3D T>
    inline TDimension3D<T> TDimension3D<T>::operator+(const TDimension3D<T>& other) const
    {
        return TDimension3D<T>(_width + other._width, _height + other._height, _depth + other._depth);
    }

    template<DimensionType3D T>
    inline TDimension3D<T> TDimension3D<T>::operator-(const TDimension3D<T>& other) const
    {
        return TDimension3D<T>(_width - other._width, _height - other._height, _depth - other._depth);
    }

    template <DimensionType3D T>
    inline T TDimension3D<T>::getArea() const
    {
        return _width * _height * _depth;
    }

    template<DimensionType3D T>
    inline bool TDimension3D<T>::isNull() const
    {
        return (_width == 0 || _height == 0 || _depth == 0);
    }

    template<DimensionType3D T>
    inline bool TDimension3D<T>::isValid() const
    {
        return (_width > 0 && _height > 0 && _depth > 0);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace math
} //namespace v3d
