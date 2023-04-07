#pragma once

#include "Vector2D.h"

namespace v3d
{
namespace math
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief TMatrix3D class
    */
    template <class T>
    class TMatrix3D
    {
    public:

        TMatrix3D() noexcept;
        TMatrix3D(const TMatrix3D<T>& other) noexcept;
        TMatrix3D(const T* matrix) noexcept;
        TMatrix3D(const TVector3D<T>& col0, const TVector3D<T>& col1, const TVector3D<T>& col2) noexcept;
        TMatrix3D(const T& m0, const T& m1, const T& m2, const T& m3, const T& m4, const T& m5, const T& m6, const T& m7, const T& m8) noexcept;
        ~TMatrix3D() = default;

        T& operator()(u32 row, u32 col);
        const T& operator()(u32 row, u32 col) const;
        T& operator[](u32 index);
        const T& operator[](u32 index) const;

        TMatrix3D<T>& operator=(const T& scalar);
        TMatrix3D<T>& operator=(const TMatrix3D<T>& other);

        [[nodiscard]] TMatrix3D<T> operator+(const TMatrix3D<T>& other) const;
        TMatrix3D<T>& operator+=(const TMatrix3D<T>& other);
        [[nodiscard]] TMatrix3D<T> operator-(const TMatrix3D<T>& other) const;
        TMatrix3D<T>& operator-=(const TMatrix3D<T>& other);
        [[nodiscard]] TMatrix3D<T> operator*(const TMatrix3D<T>& other) const;
        TMatrix3D<T>& operator*=(const TMatrix3D<T>& other);
        [[nodiscard]] TMatrix3D<T> operator*(const T& scalar) const;
        TMatrix3D<T>& operator*=(const T& scalar);
        [[nodiscard]] TVector3D<T> operator*(const TVector3D<T>& vector) const;

        bool operator==(const TMatrix3D<T>& other) const;
        bool operator!=(const TMatrix3D<T>& other) const;

        void set(const T* matrix);

        const T* getPtr() const;
        T* getPtr();

        void makeIdentity();
        [[nodiscard]] bool isIdentity() const;

        bool makeInverse();
        bool getInverse(TMatrix3D<T>& outMatrix) const;

        void makeTransposed();
        [[nodiscard]] TMatrix3D<T> getTransposed() const;

        void setTranslation(const TVector2D<T>& translation);
        [[nodiscard]] TVector2D<T> getTranslation() const;

        void setRotation(const T& angle);
        [[nodiscard]] T getRotation() const;

        void preScale(const TVector2D<T>& scale);
        void postScale(const TVector2D<T>& scale);
        void setScale(const TVector2D<T>& scale);

        [[nodiscard]] TVector2D<T> getScale() const;

    private:

        T m_matrix[9];
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template <class T>
    TMatrix3D<T> operator*(const T& scalar, const TMatrix3D<T>& matrix)
    {
        return matrix * scalar;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    using Matrix3D   = TMatrix3D<f32>;
    using Matrix3D64 = TMatrix3D<f64>;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace math
} //namespace v3d

#include "Matrix3D.inl"
