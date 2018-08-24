#include "Dimension2D.h"
namespace v3d
{
namespace core
{

    template <class T>
    TDimension2D<T>::TDimension2D()
        : width(0)
        , height(0)
    {
    }

    template <class T>
    TDimension2D<T>::TDimension2D(const T& width, const T& height)
        : width(width)
        , height(height)
    {
    }

    template <class T>
    TDimension2D<T>::TDimension2D(const TDimension2D<T>& other)
        : width(other.width)
        , height(other.height)
    {
    }

    template <class T>
    TDimension2D<T>& TDimension2D<T>::operator = (const TDimension2D<T>& other)
    {
        if (this == &other)
        {
            return *this;
        }

        width = other.width;
        height = other.height;

        return *this;
    }

    template <class T>
    bool TDimension2D<T>::operator == (const TDimension2D<T>& other) const
    {
        const bool isEquals = 
            core::isEquals(width, other.width) &&
            core::isEquals(height, other.height);

        return isEquals;
    }

    template <class T>
    bool TDimension2D<T>::operator != (const TDimension2D<T>& other) const
    {
        return !(*this == other);
    }

    template <class T>
    TDimension2D<T>& TDimension2D<T>::set(const T& nWidth, const T& nHeight)
    {
        width = nWidth;
        height = nHeight;

        return *this;
    }

    template <class T>
    TDimension2D<T>& TDimension2D<T>::operator /= (const T& scale)
    {
        width /= scale;
        height /= scale;

        return *this;
    }

    template <class T>
    TDimension2D<T> TDimension2D<T>::operator / (const T& scale) const
    {
        return TDimension2D<T>(width / scale, height / scale);
    }

    template <class T>
    TDimension2D<T>& TDimension2D<T>::operator *= (const T& scale)
    {
        width *= scale;
        height *= scale;

        return *this;
    }

    template <class T>
    TDimension2D<T> TDimension2D<T>::operator * (const T& scale) const
    {
        return TDimension2D<T>(width * scale, height * scale);
    }

    template <class T>
    TDimension2D<T>& TDimension2D<T>::operator += (const TDimension2D<T>& other)
    {
        width += other.width;
        height += other.height;

        return *this;
    }

    template <class T>
    TDimension2D<T>& TDimension2D<T>::operator -= (const TDimension2D<T>& other)
    {
        width -= other.width;
        height -= other.height;

        return *this;
    }

    template <class T>
    TDimension2D<T> TDimension2D<T>::operator+(const TDimension2D<T>& other) const
    {
        return TDimension2D<T>(width + other.width, height + other.height);
    }

    template <class T>
    inline T TDimension2D<T>::getArea() const
    {
        return width * height;
    }

    template<class T>
    inline bool TDimension2D<T>::isNull() const
    {
        return (width == 0 || height == 0);
    }

} //namespace core
} //namespace v3d
