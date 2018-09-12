#pragma once

#include "Vector2D.h"

namespace v3d
{
namespace core
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template <class T>
    class TRect
    {
    public:

        TRect();
        TRect(T x, T y, T x2, T y2);
        TRect(const TVector2D<T>& upperLeft, const TVector2D<T>& lowerRight);

        TRect<T>    operator + (const TVector2D<T>& pos) const;
        TRect<T>    operator - (const TVector2D<T>& pos) const;
        TRect<T>&   operator +=(const TVector2D<T>& pos);
        TRect<T>&   operator -=(const TVector2D<T>& pos);
        TRect<T>&   operator +=(const TRect<T>& other);

        bool        operator ==(const TRect<T>& other) const;
        bool        operator !=(const TRect<T>& other) const;
        bool        operator <(const TRect<T>& other)  const;

        void        set(T x, T y, T x2, T y2);
        void        set(const TVector2D<T>& upperLeft, const TVector2D<T>& lowerRight);

        void        scale(T scale);
        bool        equals(const TRect<T>& other, T tolerance = core::k_tolerance32) const;

        T           getArea() const;

        bool        isPointInside(const TVector2D<T>& pos)      const;
        bool        isRectCollided(const TRect<T>& other)       const;
        bool        isRectCollidedLoose(const TRect<T>& other)  const;

        void        clipAgainst(const TRect<T>& other);
        bool        constrainTo(const TRect<T>& other);
        
        Vector2D    getClosestPoint(const TVector2D<T>& pt);

        T           getWidth()      const;
        T           getHeight()     const;
        T           getLeftX()      const;
        T           getRightX()     const;
        T           getTopY()       const;
        T           getBottomY()    const;

        void        repair();

        bool        isValid()       const;
      
        TVector2D<T> getCenter()    const;
        TVector2D<T> getSize()      const;

        void        addInternalPoint(const TVector2D<T>& p);
        void        addInternalPoint(T x, T y);

    private:

        TVector2D<T> upperLeftCorner;
        TVector2D<T> lowerRightCorner;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    using Rect32  = TRect<s32>;
    using RectU32 = TRect<u32>;
    using RectF32 = TRect<f32>;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace core
} //namespace v3d

#include "Rect.inl"
