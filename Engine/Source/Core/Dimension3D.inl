#include "Dimension3D.h"
namespace v3d
{
namespace core
{

    template <class T>
    TDimension3D<T>::TDimension3D()
        : width(0)
        , height(0)
        , depth(0)
    {
    }

    template <class T>
    TDimension3D<T>::TDimension3D(const T& width, const T& height, const T& depth)
        : width(width)
        , height(height)
        , depth(depth)
    {
    }

    template <class T>
    TDimension3D<T>::TDimension3D(const TDimension3D<T>& other)
        : width(other.width)
        , height(other.height)
        , depth(other.depth)
    {
    }

    template <class T>
    TDimension3D<T>& TDimension3D<T>::operator=(const TDimension3D<T>& other)
    {
        if (this == &other)
        {
            return *this;
        }

        width = other.width;
        height = other.height;
        depth = other.depth;

        return *this;
    }

    template <class T>
    bool TDimension3D<T>::operator==(const TDimension3D<T>& other) const
    {
        const bool isEquals = 
            core::isEquals(width, other.width) &&
            core::isEquals(height, other.height) &&
            core::isEquals(depth, other.depth);

        return isEquals;
    }

    template <class T>
    bool TDimension3D<T>::operator!=(const TDimension3D<T>& other) const
    {
        return !(*this == other);
    }

    template<class T>
    inline bool TDimension3D<T>::operator>=(const TDimension3D<T>& other) const
    {
        return !(*this <= other);
    }

    template<class T>
    inline bool TDimension3D<T>::operator<=(const TDimension3D<T>& other) const
    {
        return (*this < other) || (*this == other);
    }

    template<class T>
    inline bool TDimension3D<T>::operator>(const TDimension3D<T>& other) const
    {
        return !(*this < other);
    }

    template<class T>
    inline bool TDimension3D<T>::operator<(const TDimension3D<T>& other) const
    {
        return width < other.width && height < other.height && depth < other.depth;
    }

    template <class T>
    TDimension3D<T>& TDimension3D<T>::set(const T& nWidth, const T& nHeight, const T& nDepth)
    {
        width = nWidth;
        height = nHeight;
        depth = nDepth;

        return *this;
    }

    template <class T>
    TDimension3D<T>& TDimension3D<T>::operator/=(const T& scale)
    {
        width /= scale;
        height /= scale;
        depth /= scale;

        return *this;
    }

    template <class T>
    TDimension3D<T> TDimension3D<T>::operator/(const T& scale) const
    {
        return TDimension3D<T>(width / scale, height / scale, depth / scale);
    }

    template <class T>
    TDimension3D<T>& TDimension3D<T>::operator *= (const T& scale)
    {
        width *= scale;
        height *= scale;
        depth *= scale;

        return *this;
    }

    template <class T>
    TDimension3D<T> TDimension3D<T>::operator*(const T& scale) const
    {
        return TDimension3D<T>(width * scale, height * scale, depth * scale);
    }

    template <class T>
    TDimension3D<T>& TDimension3D<T>::operator+=(const TDimension3D<T>& other)
    {
        width += other.width;
        height += other.height;
        depth += other.depth;

        return *this;
    }

    template <class T>
    TDimension3D<T>& TDimension3D<T>::operator-=(const TDimension3D<T>& other)
    {
        width -= other.width;
        height -= other.height;
        depth -= other.depth;

        return *this;
    }

    template <class T>
    TDimension3D<T> TDimension3D<T>::operator+(const TDimension3D<T>& other) const
    {
        return TDimension3D<T>(width + other.width, height + other.height, depth + other.depth);
    }

    template<class T>
    inline TDimension3D<T> TDimension3D<T>::operator-(const TDimension3D<T>& other) const
    {
        return TDimension3D<T>(width - other.width, height - other.height, depth - other.depth);
    }

    template <class T>
    T TDimension3D<T>::getArea() const
    {
        return width * height * depth;
    }

    template<class T>
    inline bool TDimension3D<T>::isNull() const
    {
        return (width == 0 || height == 0 || depth == 0);
    }

} //namespace core
} //namespace v3d
