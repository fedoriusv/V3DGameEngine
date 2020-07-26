namespace v3d
{
namespace core
{

    template <class T>
    TRect<T>::TRect()
        : upperLeftCorner(0, 0)
        , lowerRightCorner(0, 0)
    {
    }


    template <class T>
    TRect<T>::TRect(const T& x1, const T& y1, const T& x2, const T& y2)
        : upperLeftCorner(x1, y1)
        , lowerRightCorner(x2, y2)
    {
    }


    template <class T>
    TRect<T>::TRect(const TVector2D<T>& upperLeft, const TVector2D<T>& lowerRight)
        : upperLeftCorner(upperLeft)
        , lowerRightCorner(lowerRight)
    {
    }

    template<class T>
    TRect<T> TRect<T>::operator+(const TVector2D<T>& pos) const
    {
        TRect<T> ret(*this);
        return ret += pos;
    }

    template<class T>
    TRect<T>& TRect<T>::operator+=(const TVector2D<T>& pos)
    {
        upperLeftCorner += pos;
        lowerRightCorner += pos;
        return *this;
    }

    template<class T>
    void TRect<T>::set(const T& x1, const T& y1, const T& x2, const T& y2)
    {
        upperLeftCorner.Set(x1, y1);
        lowerRightCorner.Set(x2, y2);
    }

    template<class T>
    void TRect<T>::set(const TVector2D<T>& upperLeft, const TVector2D<T>& lowerRight)
    {
        upperLeftCorner = upperLeft;
        lowerRightCorner = lowerRight;
    }

    template<class T>
    TRect<T> TRect<T>::operator-(const TVector2D<T>& pos) const
    {
        TRect<T> ret(*this);
        return ret -= pos;
    }

    template<class T>
    TRect<T>& TRect<T>::operator-=(const TVector2D<T>& pos)
    {
        upperLeftCorner -= pos;
        lowerRightCorner -= pos;
        return *this;
    }

    template<class T>
    void TRect<T>::scale(T scale)
    {
        upperLeftCorner *= scale;
        lowerRightCorner *= scale;
    }

    template<class T>
    bool TRect<T>::operator==(const TRect<T>& other) const
    {
        return (upperLeftCorner == other.upperLeftCorner  &&
            lowerRightCorner == other.lowerRightCorner);
    }

    template<class T>
    bool TRect<T>::equals(const TRect<T>& other, T tolerance) const
    {
        return (upperLeftCorner.Equals(other.upperLeftCorner, tolerance) &&
            lowerRightCorner.Equals(other.lowerRightCorner, tolerance));
    }

    template<class T>
    bool TRect<T>::operator!=(const TRect<T>& other) const
    {
        return (upperLeftCorner != other.upperLeftCorner ||
            lowerRightCorner != other.lowerRightCorner);
    }

    template<class T>
    TRect<T>& TRect<T>::operator+=(const TRect<T>& other)
    {
        addInternalPoint(other.upperLeftCorner);
        addInternalPoint(other.lowerRightCorner);
        return *this;
    }

    template<class T>
    bool TRect<T>::operator<(const TRect<T>& other) const
    {
        return getArea() < other.getArea();
    }

    template<class T>
    T TRect<T>::getArea() const
    {
        return getWidth() * getHeight();
    }

    template<class T>
    bool TRect<T>::isPointInside(const TVector2D<T>& pos) const
    {
        return (upperLeftCorner.x <= pos.x &&
            upperLeftCorner.y <= pos.y &&
            lowerRightCorner.x >= pos.x &&
            lowerRightCorner.y >= pos.y);
    }

    template<class T>
    bool TRect<T>::isRectCollided(const TRect<T>& other) const
    {
        return (lowerRightCorner.y > other.upperLeftCorner.y &&
            upperLeftCorner.y  < other.lowerRightCorner.y &&
            lowerRightCorner.x > other.upperLeftCorner.x  &&
            upperLeftCorner.x  < other.lowerRightCorner.x);
    }

    template<class T>
    bool TRect<T>::isRectCollidedLoose(const TRect<T>& other) const
    {
        return (lowerRightCorner.y >(other.upperLeftCorner.y - static_cast<T>(1)) &&
            upperLeftCorner.y  < (other.lowerRightCorner.y + static_cast<T>(1)) &&
            lowerRightCorner.x >(other.upperLeftCorner.x - static_cast<T>(1)) &&
            upperLeftCorner.x < (other.lowerRightCorner.x + static_cast<T>(1)));
    }

    template<class T>
    void TRect<T>::clipAgainst(const TRect<T>& other)
    {
        if (other.lowerRightCorner.x < lowerRightCorner.x)
        {
            lowerRightCorner.x = other.lowerRightCorner.x;
        }
        if (other.lowerRightCorner.y < lowerRightCorner.y)
        {
            lowerRightCorner.y = other.lowerRightCorner.y;
        }

        if (other.upperLeftCorner.x > upperLeftCorner.x)
        {
            upperLeftCorner.x = other.upperLeftCorner.x;
        }
        if (other.upperLeftCorner.y > upperLeftCorner.y)
        {
            upperLeftCorner.y = other.mupperLeftCorner.y;
        }

        if (upperLeftCorner.y > lowerRightCorner.m_y)
        {
            upperLeftCorner.m_y = lowerRightCorner.y;
        }
        if (upperLeftCorner.x > lowerRightCorner.x)
        {
            upperLeftCorner.x = lowerRightCorner.x;
        }
    }

    template<class T>
    bool TRect<T>::constrainTo(const TRect<T>& other)
    {
        if (other.getWidth() < getWidth() || other.getHeight() < getHeight())
        {
            return false;
        }

        T diff = other.lowerRightCorner.x - lowerRightCorner.x;
        if (diff < 0)
        {
            lowerRightCorner.x += diff;
            upperLeftCorner.x += diff;
        }

        diff = other.lowerRightCorner.y - lowerRightCorner.y;
        if (diff < 0)
        {
            lowerRightCorner.y += diff;
            upperLeftCorner.y += diff;
        }

        diff = upperLeftCorner.x - other.upperLeftCorner.x;
        if (diff < 0)
        {
            upperLeftCorner.x -= diff;
            lowerRightCorner.x -= diff;
        }

        diff = upperLeftCorner.y - other.upperLeftCorner.y;
        if (diff < 0)
        {
            upperLeftCorner.y -= diff;
            lowerRightCorner.y -= diff;
        }

        return true;
    }

    template<class T>
    Vector2D TRect<T>::getClosestPoint(const TVector2D<T>& pt)
    {
        Vector2D result = pt;

        result.x = clamp(result.x, upperLeftCorner.x, lowerRightCorner.x);
        result.y = clamp(result.y, upperLeftCorner.y, lowerRightCorner.y);

        return result;
    }

    template<class T>
    T TRect<T>::getWidth() const
    {
        return lowerRightCorner.x - upperLeftCorner.x;
    }

    template<class T>
    T TRect<T>::getHeight() const
    {
        return lowerRightCorner.y - upperLeftCorner.y;
    }

    template<class T>
    T TRect<T>::getLeftX() const
    {
        return upperLeftCorner.x;
    }

    template<class T>
    T TRect<T>::getRightX() const
    {
        return lowerRightCorner.x;
    }

    template<class T>
    T TRect<T>::getTopY() const
    {
        return upperLeftCorner.y;
    }

    template<class T>
    T TRect<T>::getBottomY() const
    {
        return lowerRightCorner.y;
    }

    template<class T>
    void TRect<T>::repair()
    {
        if (lowerRightCorner.x < upperLeftCorner.x)
        {
            T t = lowerRightCorner.x;
            lowerRightCorner.x = upperLeftCorner.x;
            upperLeftCorner.x = t;
        }

        if (lowerRightCorner.y < upperLeftCorner.y)
        {
            T t = lowerRightCorner.y;
            lowerRightCorner.y = upperLeftCorner.y;
            upperLeftCorner.y = t;
        }
    }

    template<class T>
    bool TRect<T>::isValid() const
    {
        return ((lowerRightCorner.x >= upperLeftCorner.x) &&
            (lowerRightCorner.y >= upperLeftCorner.y));
    }

    template<class T>
    TVector2D<T> TRect<T>::getCenter() const
    {
        return TVector2D<T>((upperLeftCorner.x + lowerRightCorner.x) / 2,
            (upperLeftCorner.y + lowerRightCorner.y) / 2);
    }

    template<class T>
    TVector2D<T> TRect<T>::getSize() const
    {
        return TVector2D<T>(getWidth(), getHeight());
    }

    template<class T>
    void TRect<T>::addInternalPoint(const TVector2D<T>& p)
    {
        addInternalPoint(p.x, p.y);
    }

    template<class T>
    void TRect<T>::addInternalPoint(const T& x, const T& y)
    {
        if (x > lowerRightCorner.x)
        {
            lowerRightCorner.x = x;
        }
        if (y > lowerRightCorner.y)
        {
            lowerRightCorner.y = y;
        }

        if (x < upperLeftCorner.x)
        {
            upperLeftCorner.x = x;
        }
        if (y < upperLeftCorner.y)
        {
            upperLeftCorner.y = y;
        }
    }

} //namespace core
} //namespace v3d
