namespace v3d
{
namespace math
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template <class T>
    TDimension3D<T>::TDimension3D() noexcept
        : m_width(0)
        , m_height(0)
        , m_depth(0)
    {
    }

    template <class T>
    TDimension3D<T>::TDimension3D(const T& width, const T& height, const T& depth) noexcept
        : m_width(width)
        , m_height(height)
        , m_depth(depth)
    {
    }

    template <class T>
    TDimension3D<T>::TDimension3D(const TDimension3D<T>& other) noexcept
        : m_width(other.m_width)
        , m_height(other.m_height)
        , m_depth(other.m_depth)
    {
    }

    template <class T>
    inline TDimension3D<T>& TDimension3D<T>::operator=(const TDimension3D<T>& other)
    {
        if (this == &other)
        {
            return *this;
        }

        m_width = other.m_width;
        m_height = other.m_height;
        m_depth = other.m_depth;

        return *this;
    }

    template <class T>
    inline bool TDimension3D<T>::operator==(const TDimension3D<T>& other) const
    {
        const bool isEquals = 
            math::isEquals(m_width, other.m_width) &&
            math::isEquals(m_height, other.m_height) &&
            math::isEquals(m_depth, other.m_depth);

        return isEquals;
    }

    template <class T>
    inline bool TDimension3D<T>::operator!=(const TDimension3D<T>& other) const
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
        return m_width < other.m_width && m_height < other.m_height && m_depth < other.m_depth;
    }

    template <class T>
    inline TDimension3D<T>& TDimension3D<T>::set(const T& width, const T& height, const T& depth)
    {
        m_width = width;
        m_height = height;
        m_depth = depth;

        return *this;
    }

    template <class T>
    inline TDimension3D<T>& TDimension3D<T>::operator/=(const T& scale)
    {
        m_width /= scale;
        m_height /= scale;
        m_depth /= scale;

        return *this;
    }

    template <class T>
    inline TDimension3D<T> TDimension3D<T>::operator/(const T& scale) const
    {
        return TDimension3D<T>(m_width / scale, m_height / scale, m_depth / scale);
    }

    template <class T>
    inline TDimension3D<T>& TDimension3D<T>::operator *= (const T& scale)
    {
        m_width *= scale;
        m_height *= scale;
        m_depth *= scale;

        return *this;
    }

    template <class T>
    inline TDimension3D<T> TDimension3D<T>::operator*(const T& scale) const
    {
        return TDimension3D<T>(m_width * scale, m_height * scale, m_depth * scale);
    }

    template <class T>
    inline TDimension3D<T>& TDimension3D<T>::operator+=(const TDimension3D<T>& other)
    {
        m_width += other.m_width;
        m_height += other.m_height;
        m_depth += other.m_depth;

        return *this;
    }

    template <class T>
    inline TDimension3D<T>& TDimension3D<T>::operator-=(const TDimension3D<T>& other)
    {
        m_width -= other.m_width;
        m_height -= other.m_height;
        m_depth -= other.m_depth;

        return *this;
    }

    template <class T>
    inline TDimension3D<T> TDimension3D<T>::operator+(const TDimension3D<T>& other) const
    {
        return TDimension3D<T>(m_width + other.m_width, m_height + other.m_height, m_depth + other.m_depth);
    }

    template<class T>
    inline TDimension3D<T> TDimension3D<T>::operator-(const TDimension3D<T>& other) const
    {
        return TDimension3D<T>(m_width - other.m_width, m_height - other.m_height, m_depth - other.m_depth);
    }

    template <class T>
    inline T TDimension3D<T>::getArea() const
    {
        return m_width * m_height * m_depth;
    }

    template<class T>
    inline bool TDimension3D<T>::isNull() const
    {
        return (m_width == 0 || m_height == 0 || m_depth == 0);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace math
} //namespace v3d
