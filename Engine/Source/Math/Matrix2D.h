#pragma once

#include "Math.h"
#include "Vector2D.h"

namespace v3d
{
namespace math
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief TMatrix2D class
    */
    template <class T>
    class TMatrix2D
    {
    public:

        TMatrix2D() noexcept;
        TMatrix2D(const TMatrix2D<T>& other) noexcept;
        TMatrix2D(const T* matrix) noexcept;
        TMatrix2D(const T& m0, const T& m1, const T& m2, const T& m3) noexcept;
        ~TMatrix2D() = default;

        T& operator()(u32 row, u32 col);
        const T& operator()(u32 row, u32 col) const;

        T& operator[](u32 index);
        const T& operator[](u32 index) const;

        TMatrix2D<T>& operator=(const T& scalar);
        TMatrix2D<T>& operator=(const TMatrix2D<T>& other);

        [[nodiscard]] TMatrix2D<T> operator+(const TMatrix2D<T>& other) const;
        TMatrix2D<T>& operator+=(const TMatrix2D<T>& other);
        [[nodiscard]] TMatrix2D<T> operator-(const TMatrix2D<T>& other) const;
        TMatrix2D<T>& operator-=(const TMatrix2D<T>& other);
        [[nodiscard]] TMatrix2D<T> operator*(const TMatrix2D<T>& other) const;
        TMatrix2D<T>& operator*=(const TMatrix2D<T>& other);
        [[nodiscard]] TMatrix2D<T> operator*(const T& scalar) const;
        TMatrix2D<T>& operator*=(const T& scalar);

        bool operator==(const TMatrix2D<T>& other) const;
        bool operator!=(const TMatrix2D<T>& other) const;

        TMatrix2D<T>& set(const T* data);

        T* getPtr();
        const T* getPtr() const;

        void makeIdentity();
        bool isIdentity() const;

        void setRotation(T angle);
        [[nodiscard]] T getRotation() const;

        void preScale(const TVector2D<T>& scale);
        void postScale(const TVector2D<T>& scale);
        void setScale(const TVector2D<T>& scale);

        [[nodiscard]] TVector2D<T> getScale() const;

        bool makeInverse();
        bool getInverse(TMatrix2D<T>& out) const;

        void makeTransposed();
        [[nodiscard]] TMatrix2D<T> getTransposed() const;

    private:

        T m_matrix[4];
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template <class T>
    TMatrix2D<T> operator*(const T& scalar, const TMatrix2D<T>& matrix)
    {
        return matrix * scalar;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    using Matrix2D = TMatrix2D<f32>;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace math
} //namespace v3d

#include "Matrix2D.inl"
