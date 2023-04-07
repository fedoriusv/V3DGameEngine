namespace v3d
{
namespace math
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template <class T>
    TRect<T>::TRect() noexcept
        : m_upperLeftCorner(0, 0)
        , m_lowerRightCorner(0, 0)
    {
    }


    template <class T>
    TRect<T>::TRect(const T& x1, const T& y1, const T& x2, const T& y2) noexcept
        : m_upperLeftCorner(x1, y1)
        , m_lowerRightCorner(x2, y2)
    {
    }


    template <class T>
    TRect<T>::TRect(const TVector2D<T>& upperLeft, const TVector2D<T>& lowerRight) noexcept
        : m_upperLeftCorner(upperLeft)
        , m_lowerRightCorner(lowerRight)
    {
    }

    template<class T>
    inline TRect<T> TRect<T>::operator+(const TVector2D<T>& pos) const
    {
        TRect<T> ret(*this);
        return ret += pos;
    }

    template<class T>
    inline TRect<T>& TRect<T>::operator+=(const TVector2D<T>& pos)
    {
        m_upperLeftCorner += pos;
        m_lowerRightCorner += pos;
        return *this;
    }

    template<class T>
    inline void TRect<T>::set(const T& x1, const T& y1, const T& x2, const T& y2)
    {
        m_upperLeftCorner.Set(x1, y1);
        m_lowerRightCorner.Set(x2, y2);
    }

    template<class T>
    inline void TRect<T>::set(const TVector2D<T>& upperLeft, const TVector2D<T>& lowerRight)
    {
        m_upperLeftCorner = upperLeft;
        m_lowerRightCorner = lowerRight;
    }

    template<class T>
    inline TRect<T> TRect<T>::operator-(const TVector2D<T>& pos) const
    {
        TRect<T> ret(*this);
        return ret -= pos;
    }

    template<class T>
    inline TRect<T>& TRect<T>::operator-=(const TVector2D<T>& pos)
    {
        m_upperLeftCorner -= pos;
        m_lowerRightCorner -= pos;
        return *this;
    }

    template<class T>
    inline void TRect<T>::scale(T scale)
    {
        m_upperLeftCorner *= scale;
        m_lowerRightCorner *= scale;
    }

    template<class T>
    inline bool TRect<T>::operator==(const TRect<T>& other) const
    {
        return (m_upperLeftCorner == other.m_upperLeftCorner && m_lowerRightCorner == other.lowerRightCorner);
    }

    template<class T>
    inline bool TRect<T>::equals(const TRect<T>& other, T tolerance) const
    {
        return (m_upperLeftCorner.Equals(other.m_upperLeftCorner, tolerance) && m_lowerRightCorner.Equals(other.m_lowerRightCorner, tolerance));
    }

    template<class T>
    inline bool TRect<T>::operator!=(const TRect<T>& other) const
    {
        return (m_upperLeftCorner != other.m_upperLeftCorner || m_lowerRightCorner != other.m_lowerRightCorner);
    }

    template<class T>
    inline TRect<T>& TRect<T>::operator+=(const TRect<T>& other)
    {
        addInternalPoint(other.m_upperLeftCorner);
        addInternalPoint(other.m_lowerRightCorner);
        return *this;
    }

    template<class T>
    inline bool TRect<T>::operator<(const TRect<T>& other) const
    {
        return getArea() < other.getArea();
    }

    template<class T>
    inline T TRect<T>::getArea() const
    {
        return getWidth() * getHeight();
    }

    template<class T>
    inline bool TRect<T>::isPointInside(const TVector2D<T>& pos) const
    {
        return (m_upperLeftCorner.m_x <= pos.m_x && m_upperLeftCorner.m_y <= pos.m_y && m_lowerRightCorner.m_x >= pos.m_x && m_lowerRightCorner.m_y >= pos.m_y);
    }

    template<class T>
    inline bool TRect<T>::isRectCollided(const TRect<T>& other) const
    {
        return (m_lowerRightCorner.m_y > other.m_upperLeftCorner.m_y && m_upperLeftCorner.m_y  < other.m_lowerRightCorner.m_y &&
            m_lowerRightCorner.m_x > other.m_upperLeftCorner.m_x && m_upperLeftCorner.m_x  < other.m_lowerRightCorner.m_x);
    }

    template<class T>
    inline bool TRect<T>::isRectCollidedLoose(const TRect<T>& other) const
    {
        return (m_lowerRightCorner.m_y >(other.m_upperLeftCorner.m_y - static_cast<T>(1)) && m_upperLeftCorner.m_y  < (other.m_lowerRightCorner.m_y + static_cast<T>(1)) &&
            m_lowerRightCorner.m_x >(other.m_upperLeftCorner.m_x - static_cast<T>(1)) && m_upperLeftCorner.m_x < (other.m_lowerRightCorner.m_x + static_cast<T>(1)));
    }

    template<class T>
    inline void TRect<T>::clipAgainst(const TRect<T>& other)
    {
        if (other.m_lowerRightCorner.m_x < m_lowerRightCorner.m_x)
        {
            m_lowerRightCorner.m_x = other.m_lowerRightCorner.m_x;
        }
        if (other.m_lowerRightCorner.m_y < m_lowerRightCorner.m_y)
        {
            m_lowerRightCorner.m_y = other.m_lowerRightCorner.m_y;
        }

        if (other.m_upperLeftCorner.m_x > m_upperLeftCorner.m_x)
        {
            m_upperLeftCorner.m_x = other.m_upperLeftCorner.m_x;
        }
        if (other.m_upperLeftCorner.m_y > m_upperLeftCorner.m_y)
        {
            m_upperLeftCorner.m_y = other.m_upperLeftCorner.m_y;
        }

        if (m_upperLeftCorner.m_y > m_lowerRightCorner.m_y)
        {
            m_upperLeftCorner.m_y = m_lowerRightCorner.m_y;
        }
        if (m_upperLeftCorner.m_x > m_lowerRightCorner.m_x)
        {
            m_upperLeftCorner.m_x = m_lowerRightCorner.m_x;
        }
    }

    template<class T>
    inline bool TRect<T>::constrainTo(const TRect<T>& other)
    {
        if (other.getWidth() < getWidth() || other.getHeight() < getHeight())
        {
            return false;
        }

        T diff = other.m_lowerRightCorner.m_x - m_lowerRightCorner.m_x;
        if (diff < 0)
        {
            m_lowerRightCorner.m_x += diff;
            m_upperLeftCorner.m_x += diff;
        }

        diff = other.m_lowerRightCorner.m_y - m_lowerRightCorner.m_y;
        if (diff < 0)
        {
            m_lowerRightCorner.m_y += diff;
            m_upperLeftCorner.m_y += diff;
        }

        diff = m_upperLeftCorner.m_x - other.m_upperLeftCorner.m_x;
        if (diff < 0)
        {
            m_upperLeftCorner.m_x -= diff;
            m_lowerRightCorner.m_x -= diff;
        }

        diff = m_upperLeftCorner.m_y - other.m_upperLeftCorner.m_y;
        if (diff < 0)
        {
            m_upperLeftCorner.m_y -= diff;
            m_lowerRightCorner.m_y -= diff;
        }

        return true;
    }

    template<class T>
    inline Vector2D TRect<T>::getClosestPoint(const TVector2D<T>& pt)
    {
        Vector2D result = pt;

        result.m_x = math::clamp(result.m_x, m_upperLeftCorner.m_x, m_lowerRightCorner.m_x);
        result.m_y = math::clamp(result.m_y, m_upperLeftCorner.m_y, m_lowerRightCorner.m_y);

        return result;
    }

    template<class T>
    inline T TRect<T>::getWidth() const
    {
        return m_lowerRightCorner.m_x - m_upperLeftCorner.m_x;
    }

    template<class T>
    inline T TRect<T>::getHeight() const
    {
        return m_lowerRightCorner.m_y - m_upperLeftCorner.m_y;
    }

    template<class T>
    inline T TRect<T>::getLeftX() const
    {
        return m_upperLeftCorner.m_x;
    }

    template<class T>
    inline T TRect<T>::getRightX() const
    {
        return m_lowerRightCorner.m_x;
    }

    template<class T>
    inline T TRect<T>::getTopY() const
    {
        return m_upperLeftCorner.m_y;
    }

    template<class T>
    inline T TRect<T>::getBottomY() const
    {
        return m_lowerRightCorner.m_y;
    }

    template<class T>
    inline void TRect<T>::repair()
    {
        if (m_lowerRightCorner.m_x < m_upperLeftCorner.m_x)
        {
            T t = m_lowerRightCorner.m_x;
            m_lowerRightCorner.m_x = m_upperLeftCorner.m_x;
            m_upperLeftCorner.m_x = t;
        }

        if (m_lowerRightCorner.m_y < m_upperLeftCorner.m_y)
        {
            T t = m_lowerRightCorner.m_y;
            m_lowerRightCorner.m_y = m_upperLeftCorner.m_y;
            m_upperLeftCorner.m_y = t;
        }
    }

    template<class T>
    inline bool TRect<T>::isValid() const
    {
        return ((m_lowerRightCorner.m_x >= m_upperLeftCorner.m_x) && (m_lowerRightCorner.m_y >= m_upperLeftCorner.m_y));
    }

    template<class T>
    inline TVector2D<T> TRect<T>::getCenter() const
    {
        return TVector2D<T>((m_upperLeftCorner.m_x + m_lowerRightCorner.m_x) / 2,
            (m_upperLeftCorner.m_y + m_lowerRightCorner.m_y) / 2);
    }

    template<class T>
    inline TVector2D<T> TRect<T>::getSize() const
    {
        return TVector2D<T>(getWidth(), getHeight());
    }

    template<class T>
    inline void TRect<T>::addInternalPoint(const TVector2D<T>& p)
    {
        addInternalPoint(p.m_x, p.m_y);
    }

    template<class T>
    inline void TRect<T>::addInternalPoint(const T& x, const T& y)
    {
        if (x > m_lowerRightCorner.m_x)
        {
            m_lowerRightCorner.m_x = x;
        }
        if (y > m_lowerRightCorner.m_y)
        {
            m_lowerRightCorner.m_y = y;
        }

        if (x < m_upperLeftCorner.m_x)
        {
            m_upperLeftCorner.m_x = x;
        }
        if (y < m_upperLeftCorner.m_y)
        {
            m_upperLeftCorner.m_y = y;
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace math
} //namespace v3d
