#include "Dimension2D.h"

namespace v3d
{
namespace math
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template <DimensionType2D T>
    TDimension2D<T>::TDimension2D() noexcept
        : _width(0)
        , _height(0)
    {
    }

    template <DimensionType2D T>
    TDimension2D<T>::TDimension2D(T width, T height) noexcept
        : _width(width)
        , _height(height)
    {
    }

    template <DimensionType2D T>
    TDimension2D<T>::TDimension2D(const TDimension2D<T>& other) noexcept
        : _width(other._width)
        , _height(other._height)
    {
    }

    template <DimensionType2D T>
    inline TDimension2D<T>& TDimension2D<T>::operator=(const TDimension2D<T>& other)
    {
        if (this == &other)
        {
            return *this;
        }

        _width = other._width;
        _height = other._height;

        return *this;
    }

    template <DimensionType2D T>
    inline bool TDimension2D<T>::operator==(const TDimension2D<T>& other) const
    {
        const bool isEquals = 
            math::isEquals(_width, other._width) &&
            math::isEquals(_height, other._height);

        return isEquals;
    }

    template <DimensionType2D T>
    inline bool TDimension2D<T>::operator!=(const TDimension2D<T>& other) const
    {
        return !(*this == other);
    }

    template <DimensionType2D T>
    inline TDimension2D<T>& TDimension2D<T>::set(T width, T height)
    {
        _width = width;
        _height = height;

        return *this;
    }

    template <DimensionType2D T>
    inline TDimension2D<T>& TDimension2D<T>::operator/=(T scale)
    {
        _width /= scale;
        _height /= scale;

        return *this;
    }

    template <DimensionType2D T>
    inline TDimension2D<T> TDimension2D<T>::operator/(T scale) const
    {
        return TDimension2D<T>(_width / scale, _height / scale);
    }

    template <DimensionType2D T>
    inline TDimension2D<T>& TDimension2D<T>::operator*=(T scale)
    {
        _width *= scale;
        _height *= scale;

        return *this;
    }

    template <DimensionType2D T>
    inline TDimension2D<T> TDimension2D<T>::operator*(T scale) const
    {
        return TDimension2D<T>(_width * scale, _height * scale);
    }

    template <DimensionType2D T>
    inline TDimension2D<T>& TDimension2D<T>::operator+=(const TDimension2D<T>& other)
    {
        _width += other._width;
        _height += other._height;

        return *this;
    }

    template <DimensionType2D T>
    inline TDimension2D<T> TDimension2D<T>::operator-(const TDimension2D<T>& other) const
    {
        return TDimension2D<T>(_width - other._width, _height - other._height);
    }

    template <DimensionType2D T>
    inline TDimension2D<T>& TDimension2D<T>::operator-=(const TDimension2D<T>& other)
    {
        _width -= other._width;
        _height -= other._height;

        return *this;
    }

    template <DimensionType2D T>
    inline TDimension2D<T> TDimension2D<T>::operator+(const TDimension2D<T>& other) const
    {
        return TDimension2D<T>(_width + other._width, _height + other._height);
    }

    template <DimensionType2D T>
    inline T TDimension2D<T>::getArea() const
    {
        return _width * _height;
    }

    template<DimensionType2D T>
    inline bool TDimension2D<T>::isNull() const
    {
        return (_width == 0 || _height == 0);
    }

    template<DimensionType2D T>
    inline bool TDimension2D<T>::isValid() const
    {
        return (_width > 0 && _height > 0);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace math
} //namespace v3d
