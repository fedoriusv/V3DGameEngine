namespace v3d
{
namespace math
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template <typename T>
    constexpr inline TRect<T>::TRect() noexcept
        : _upperLeftCorner(0, 0)
        , _lowerRightCorner(0, 0)
    {
    }

    template <typename T>
    constexpr inline TRect<T>::TRect(T x1, T y1, T x2, T y2) noexcept
        : _upperLeftCorner(x1, y1)
        , _lowerRightCorner(x2, y2)
    {
    }

    template <typename T>
    constexpr inline TRect<T>::TRect(const TVector2D<T>& upperLeft, const TVector2D<T>& lowerRight) noexcept
        : _upperLeftCorner(upperLeft)
        , _lowerRightCorner(lowerRight)
    {
    }

    template<typename T>
    inline TRect<T> TRect<T>::operator+(const TVector2D<T>& pos) const
    {
        TRect<T> ret(*this);
        return ret += pos;
    }

    template<typename T>
    inline TRect<T>& TRect<T>::operator+=(const TVector2D<T>& pos)
    {
        _upperLeftCorner += pos;
        _lowerRightCorner += pos;
        return *this;
    }

    template<typename T>
    inline void TRect<T>::set(T x1, T y1, T x2, T y2)
    {
        _upperLeftCorner.set(x1, y1);
        _lowerRightCorner.set(x2, y2);
    }

    template<typename T>
    inline void TRect<T>::set(const TVector2D<T>& upperLeft, const TVector2D<T>& lowerRight)
    {
        _upperLeftCorner = upperLeft;
        _lowerRightCorner = lowerRight;
    }

    template<typename T>
    inline TRect<T> TRect<T>::operator-(const TVector2D<T>& pos) const
    {
        TRect<T> ret(*this);
        return ret -= pos;
    }

    template<typename T>
    inline TRect<T>& TRect<T>::operator-=(const TVector2D<T>& pos)
    {
        _upperLeftCorner -= pos;
        _lowerRightCorner -= pos;
        return *this;
    }

    template<typename T>
    inline void TRect<T>::scale(T scale)
    {
        _upperLeftCorner *= scale;
        _lowerRightCorner *= scale;
    }

    template<typename T>
    inline bool TRect<T>::operator==(const TRect<T>& other) const
    {
        return (_upperLeftCorner == other._upperLeftCorner && _lowerRightCorner == other.lowerRightCorner);
    }

    template<typename T>
    inline bool TRect<T>::equals(const TRect<T>& other, T tolerance) const
    {
        return (_upperLeftCorner.Equals(other._upperLeftCorner, tolerance) && _lowerRightCorner.Equals(other._lowerRightCorner, tolerance));
    }

    template<typename T>
    inline bool TRect<T>::operator!=(const TRect<T>& other) const
    {
        return (_upperLeftCorner != other._upperLeftCorner || _lowerRightCorner != other._lowerRightCorner);
    }

    template<typename T>
    inline TRect<T>& TRect<T>::operator+=(const TRect<T>& other)
    {
        addInternalPoint(other._upperLeftCorner);
        addInternalPoint(other._lowerRightCorner);
        return *this;
    }

    template<typename T>
    inline bool TRect<T>::operator<(const TRect<T>& other) const
    {
        return getArea() < other.getArea();
    }

    template<typename T>
    inline T TRect<T>::getArea() const
    {
        return getWidth() * getHeight();
    }

    template<typename T>
    inline bool TRect<T>::isPointInside(const TVector2D<T>& pos) const
    {
        return (_upperLeftCorner._x <= pos._x && _upperLeftCorner._y <= pos._y && _lowerRightCorner._x >= pos._x && _lowerRightCorner._y >= pos._y);
    }

    template<typename T>
    inline bool TRect<T>::isRectCollided(const TRect<T>& other) const
    {
        return (_lowerRightCorner._y > other._upperLeftCorner._y && _upperLeftCorner._y  < other._lowerRightCorner._y &&
            _lowerRightCorner._x > other._upperLeftCorner._x && _upperLeftCorner._x  < other._lowerRightCorner._x);
    }

    template<typename T>
    inline bool TRect<T>::isRectCollidedLoose(const TRect<T>& other) const
    {
        return (_lowerRightCorner._y >(other._upperLeftCorner._y - static_cast<T>(1)) && _upperLeftCorner._y  < (other._lowerRightCorner._y + static_cast<T>(1)) &&
            _lowerRightCorner._x >(other._upperLeftCorner._x - static_cast<T>(1)) && _upperLeftCorner._x < (other._lowerRightCorner._x + static_cast<T>(1)));
    }

    template<typename T>
    inline void TRect<T>::clipAgainst(const TRect<T>& other)
    {
        if (other._lowerRightCorner._x < _lowerRightCorner._x)
        {
            _lowerRightCorner._x = other._lowerRightCorner._x;
        }
        if (other._lowerRightCorner._y < _lowerRightCorner._y)
        {
            _lowerRightCorner._y = other._lowerRightCorner._y;
        }

        if (other._upperLeftCorner._x > _upperLeftCorner._x)
        {
            _upperLeftCorner._x = other._upperLeftCorner._x;
        }
        if (other._upperLeftCorner._y > _upperLeftCorner._y)
        {
            _upperLeftCorner._y = other._upperLeftCorner._y;
        }

        if (_upperLeftCorner._y > _lowerRightCorner._y)
        {
            _upperLeftCorner._y = _lowerRightCorner._y;
        }
        if (_upperLeftCorner._x > _lowerRightCorner._x)
        {
            _upperLeftCorner._x = _lowerRightCorner._x;
        }
    }

    template<typename T>
    inline bool TRect<T>::constrainTo(const TRect<T>& other)
    {
        if (other.getWidth() < getWidth() || other.getHeight() < getHeight())
        {
            return false;
        }

        T diff = other._lowerRightCorner._x - _lowerRightCorner._x;
        if (diff < 0)
        {
            _lowerRightCorner._x += diff;
            _upperLeftCorner._x += diff;
        }

        diff = other._lowerRightCorner._y - _lowerRightCorner._y;
        if (diff < 0)
        {
            _lowerRightCorner.y += diff;
            _upperLeftCorner.y += diff;
        }

        diff = _upperLeftCorner._x - other._upperLeftCorner._x;
        if (diff < 0)
        {
            _upperLeftCorner._x -= diff;
            _lowerRightCorner._x -= diff;
        }

        diff = _upperLeftCorner._y - other._upperLeftCorner._y;
        if (diff < 0)
        {
            _upperLeftCorner._y -= diff;
            _lowerRightCorner._y -= diff;
        }

        return true;
    }

    template<typename T>
    inline TVector2D<T> TRect<T>::getClosestPoint(const TVector2D<T>& pt)
    {
        TVector2D<T> result(pt);
        result._x = std::clamp(result._x, _upperLeftCorner._x, _lowerRightCorner._x);
        result._y = std::clamp(result._y, _upperLeftCorner._y, _lowerRightCorner._y);
        return result;
    }

    template<typename T>
    inline T TRect<T>::getWidth() const
    {
        return _lowerRightCorner._x - _upperLeftCorner._x;
    }

    template<typename T>
    inline T TRect<T>::getHeight() const
    {
        return _lowerRightCorner._y - _upperLeftCorner._y;
    }

    template<typename T>
    inline T TRect<T>::getLeftX() const
    {
        return _upperLeftCorner._x;
    }

    template<typename T>
    inline T TRect<T>::getRightX() const
    {
        return _lowerRightCorner._x;
    }

    template<typename T>
    inline T TRect<T>::getTopY() const
    {
        return _upperLeftCorner._y;
    }

    template<typename T>
    inline T TRect<T>::getBottomY() const
    {
        return _lowerRightCorner._y;
    }

    template<typename T>
    inline void TRect<T>::repair()
    {
        if (_lowerRightCorner._x < _upperLeftCorner._x)
        {
            T t = _lowerRightCorner._x;
            _lowerRightCorner._x = _upperLeftCorner._x;
            _upperLeftCorner._x = t;
        }

        if (_lowerRightCorner.y < _upperLeftCorner._y)
        {
            T t = _lowerRightCorner._y;
            _lowerRightCorner._y = _upperLeftCorner._y;
            _upperLeftCorner._y = t;
        }
    }

    template<typename T>
    inline bool TRect<T>::isValid() const
    {
        return ((_lowerRightCorner._x >= _upperLeftCorner._x) && (_lowerRightCorner._y >= _upperLeftCorner._y));
    }

    template<typename T>
    inline TVector2D<T> TRect<T>::getCenter() const
    {
        return TVector2D<T>((_upperLeftCorner._x + _lowerRightCorner._x) / 2,
            (_upperLeftCorner._y + _lowerRightCorner._y) / 2);
    }

    template<typename T>
    inline TVector2D<T> TRect<T>::getSize() const
    {
        return TVector2D<T>(getWidth(), getHeight());
    }

    template<typename T>
    inline void TRect<T>::addInternalPoint(const TVector2D<T>& p)
    {
        addInternalPoint(p._x, p._y);
    }

    template<typename T>
    inline void TRect<T>::addInternalPoint(const T& x, const T& y)
    {
        if (x > _lowerRightCorner._x)
        {
            _lowerRightCorner._x = x;
        }
        if (y > _lowerRightCorner._y)
        {
            _lowerRightCorner.y = y;
        }

        if (x < _upperLeftCorner._x)
        {
            _upperLeftCorner._x = x;
        }
        if (y < _upperLeftCorner._y)
        {
            _upperLeftCorner._y = y;
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace math
} //namespace v3d
