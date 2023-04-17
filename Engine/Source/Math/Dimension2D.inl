namespace v3d
{
namespace math
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template <class T>
    TDimension2D<T>::TDimension2D() noexcept
        : m_width(0)
        , m_height(0)
    {
    }

    template <class T>
    TDimension2D<T>::TDimension2D(const T& width, const T& height) noexcept
        : m_width(width)
        , m_height(height)
    {
    }

    template <class T>
    TDimension2D<T>::TDimension2D(const TDimension2D<T>& other) noexcept
        : m_width(other.m_width)
        , m_height(other.m_height)
    {
    }

    template <class T>
    inline TDimension2D<T>& TDimension2D<T>::operator=(const TDimension2D<T>& other)
    {
        if (this == &other)
        {
            return *this;
        }

        m_width = other.m_width;
        m_height = other.m_height;

        return *this;
    }

    template <class T>
    inline bool TDimension2D<T>::operator==(const TDimension2D<T>& other) const
    {
        const bool isEquals = 
            math::isEquals(m_width, other.m_width) &&
            math::isEquals(m_height, other.m_height);

        return isEquals;
    }

    template <class T>
    inline bool TDimension2D<T>::operator!=(const TDimension2D<T>& other) const
    {
        return !(*this == other);
    }

    template <class T>
    inline TDimension2D<T>& TDimension2D<T>::set(const T& width, const T& height)
    {
        m_width = width;
        m_height = height;

        return *this;
    }

    template <class T>
    inline TDimension2D<T>& TDimension2D<T>::operator/=(const T& scale)
    {
        m_width /= scale;
        m_height /= scale;

        return *this;
    }

    template <class T>
    inline TDimension2D<T> TDimension2D<T>::operator/(const T& scale) const
    {
        return TDimension2D<T>(m_width / scale, m_height / scale);
    }

    template <class T>
    inline TDimension2D<T>& TDimension2D<T>::operator*=(const T& scale)
    {
        m_width *= scale;
        m_height *= scale;

        return *this;
    }

    template <class T>
    inline TDimension2D<T> TDimension2D<T>::operator*(const T& scale) const
    {
        return TDimension2D<T>(m_width * scale, m_height * scale);
    }

    template <class T>
    inline TDimension2D<T>& TDimension2D<T>::operator+=(const TDimension2D<T>& other)
    {
        m_width += other.m_width;
        m_height += other.m_height;

        return *this;
    }

    template <class T>
    inline TDimension2D<T> TDimension2D<T>::operator-(const TDimension2D<T>& other) const
    {
        return TDimension2D<T>(m_width - other.m_width, m_height - other.m_height);
    }

    template <class T>
    inline TDimension2D<T>& TDimension2D<T>::operator-=(const TDimension2D<T>& other)
    {
        m_width -= other.m_width;
        m_height -= other.m_height;

        return *this;
    }

    template <class T>
    inline TDimension2D<T> TDimension2D<T>::operator+(const TDimension2D<T>& other) const
    {
        return TDimension2D<T>(m_width + other.m_width, m_height + other.m_height);
    }

    template <class T>
    inline T TDimension2D<T>::getArea() const
    {
        return m_width * m_height;
    }

    template<class T>
    inline bool TDimension2D<T>::isNull() const
    {
        return (m_width == 0 || m_height == 0);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace math
} //namespace v3d
