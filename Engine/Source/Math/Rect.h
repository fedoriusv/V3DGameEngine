#pragma once

#include "Math.h"
#include "Vector2D.h"

namespace v3d
{
namespace math
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief TRect struct
    */
    template <typename T>
    struct TRect
    {
    public:

        TRect() noexcept;
        TRect(T x1, T y1, T x2, T y2) noexcept;
        TRect(const TVector2D<T>& upperLeft, const TVector2D<T>& lowerRight) noexcept;
        ~TRect() noexcept = default;

        [[nodiscard]] TRect<T> operator+(const TVector2D<T>& pos) const;
        TRect<T>& operator+=(const TVector2D<T>& pos);
        TRect<T>& operator+=(const TRect<T>& other);
        [[nodiscard]] TRect<T> operator-(const TVector2D<T>& pos) const;
        TRect<T>& operator-=(const TVector2D<T>& pos);

        bool operator==(const TRect<T>& other) const;
        bool operator!=(const TRect<T>& other) const;
        bool operator<(const TRect<T>& other) const;

        void set(T x1, T y1, T x2, T y2);
        void set(const TVector2D<T>& upperLeft, const TVector2D<T>& lowerRight);

        void scale(T scale);
        [[nodiscard]] bool equals(const TRect<T>& other, T tolerance = core::k_tolerance32) const;

        [[nodiscard]] T getArea() const;

        [[nodiscard]] bool isPointInside(const TVector2D<T>& pos) const;
        [[nodiscard]] bool isRectCollided(const TRect<T>& other) const;
        [[nodiscard]] bool isRectCollidedLoose(const TRect<T>& other) const;

        void clipAgainst(const TRect<T>& other);
        [[nodiscard]] bool constrainTo(const TRect<T>& other);

        [[nodiscard]] TVector2D<T> getClosestPoint(const TVector2D<T>& point);

        [[nodiscard]] T getWidth() const;
        [[nodiscard]] T getHeight() const;
        [[nodiscard]] T getLeftX() const;
        [[nodiscard]] T getRightX() const;
        [[nodiscard]] T getTopY() const;
        [[nodiscard]] T getBottomY() const;

        void repair();

        bool isValid() const;
      
        [[nodiscard]] TVector2D<T> getCenter() const;
        [[nodiscard]] TVector2D<T> getSize() const;

        void addInternalPoint(const TVector2D<T>& point);
        void addInternalPoint(const T& x, const T& y);

    private:

        TVector2D<T> _upperLeftCorner;
        TVector2D<T> _lowerRightCorner;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    using Rect = TRect<f32>;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace math
} //namespace v3d

#include "Rect.inl"
