#pragma once

#include "Vector3D.h"
#include "Vector4D.h"

namespace v3d
{
namespace math
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Reserved Z
    */
#   define REVERSED_Z 1

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief TMatrix4D class
    */
    template <class T>
    class TMatrix4D
    {
    public:

        TMatrix4D() noexcept;
        TMatrix4D(const TMatrix4D<T>& other) noexcept;
        TMatrix4D(const TVector4D<T>& col0, const TVector4D<T>& col1, const TVector4D<T>& col2, const TVector4D<T>& col3) noexcept;
        TMatrix4D(const T* matrix) noexcept;
        TMatrix4D(const T& m0, const T& m1, const T& m2, const T& m3,
            const T& m4, const T& m5, const T& m6, const T& m7,
            const T& m8, const T& m9, const T& m10, const T& m11,
            const T& m12, const T& m13, const T& m14, const T& m15) noexcept;
        ~TMatrix4D() = default;

        T& operator()(u32 row, u32 col);
        const T& operator()(u32 row, u32 col) const;
        T& operator[](u32 index);
        const T& operator[](u32 index) const;

        TMatrix4D<T>& operator=(const T& scalar);
        TMatrix4D<T>& operator=(const TMatrix4D<T>& other);

        [[nodiscard]] TMatrix4D<T> operator+(const TMatrix4D<T>& other) const;
        TMatrix4D<T>& operator+=(const TMatrix4D<T>& other);
        [[nodiscard]] TMatrix4D<T> operator-(const TMatrix4D<T>& other) const;
        TMatrix4D<T>& operator-=(const TMatrix4D<T>& other);
        [[nodiscard]] TMatrix4D<T> operator*(const TMatrix4D<T>& other) const;
        TMatrix4D<T>& operator*=(const TMatrix4D<T>& other);
        [[nodiscard]] TMatrix4D<T> operator*(const T& scalar) const;
        TMatrix4D<T>& operator*=(const T& scalar);
        [[nodiscard]] TVector4D<T> operator*(const TVector4D<T>& vector) const;

        bool operator==(const TMatrix4D<T>& other) const;
        bool operator!=(const TMatrix4D<T>& other) const;

        void set(const T* matrix);

        T* getPtr();
        const T* getPtr() const;

        void makeIdentity();
        [[nodiscard]] bool isIdentity() const;

        void makeTransposed();
        [[nodiscard]] TMatrix4D<T> getTransposed() const;

        void setTranslation(const TVector3D<T>& translation);
        [[nodiscard]] TVector3D<T> getTranslation() const;

        void setRotation(const TVector3D<T>& rotation);
        [[nodiscard]] TVector3D<T> getRotation() const;

        void preScale(const TVector3D<T>& scale);
        void postScale(const TVector3D<T>& scale);
        void setScale(const TVector3D<T>& scale);

        [[nodiscard]] TVector3D<T> getScale() const;

        bool makeInverse();
        bool getInverse(TMatrix4D<T>& outMatrix) const;

    private:

        T m_matrix[16];
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template <class T>
    TMatrix4D<T> operator*(const T& scalar, const TMatrix4D<T>& matrix)
    {
        return matrix * scalar;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    using Matrix4D = TMatrix4D<f32>;
    using Matrix4D64 = TMatrix4D<f64>;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    inline Matrix4D buildProjectionMatrixPerspective(f32 fieldOfView, f32 aspectRatio, f32 zNear, f32 zFar)
    {
        Matrix4D outMatrix;
        f32* matrix = outMatrix.getPtr();

        const f32 yFac = tanf(fieldOfView * k_degToRad / 2.f);
        const f32 xFac = yFac * aspectRatio;

        matrix[0] = 1.f / xFac;
        matrix[1] = 0.f;
        matrix[2] = 0.f;
        matrix[3] = 0.f;

        matrix[4] = 0.f;
        matrix[5] = 1.f / yFac;
        matrix[6] = 0.f;
        matrix[7] = 0.f;

        matrix[8] = 0.f;
        matrix[9] = 0.f;
#if REVERSED_Z
        matrix[10] = zNear / (zNear - zFar);
#else
        matrix[10] = zFar / (zFar - zNear);
#endif //REVERSED_Z
        matrix[11] = 1.f;

        matrix[12] = 0.f;
        matrix[13] = 0.f;
#if REVERSED_Z
        matrix[14] = -(zFar * zNear) / (zNear - zFar);
#else
        matrix[14] = -(zFar * zNear) / (zFar - zNear);
#endif //REVERSED_Z
        matrix[15] = 0.f;

        return outMatrix;
    }

    inline Matrix4D buildProjectionMatrixFrustum(f32 left, f32 right, f32 bottom, f32 top, f32 zNear, f32 zFar)
    {
        Matrix4D outMatrix;
        f32* matrix = outMatrix.getPtr();
#if REVERSED_Z
        matrix[0] = 2.f * zFar / (right - left);
#else
        matrix[0] = 2.f * zNear / (right - left);
#endif //REVERSED_Z
        matrix[1] = 0.f;
        matrix[2] = 0.f;
        matrix[3] = 0.f;

        matrix[4] = 0.f;
#if REVERSED_Z
        matrix[5] = 2.f * zFar / (top - bottom);
#else
        matrix[5] = 2.f * zNear / (top - bottom);
#endif //REVERSED_Z
        matrix[6] = 0.f;
        matrix[7] = 0.f;

        matrix[8] = (right + left) / (right - left);
        matrix[9] = (top + bottom) / (top - bottom);
#if REVERSED_Z
        matrix[10] = zNear / (zNear - zFar);
#else
        matrix[10] = zFar / (zFar - zNear);
#endif //REVERSED_Z
        matrix[11] = 1.f;

        matrix[12] = 0.f;
        matrix[13] = 0.f;
#if REVERSED_Z
        matrix[14] = -(zFar * zNear) / (zNear - zFar);
#else
        matrix[14] = -(zFar * zNear) / (zFar - zNear);
#endif //REVERSED_Z
        matrix[15] = 1.f;

        return outMatrix;
    }

    inline Matrix4D buildProjectionMatrixOrtho(f32 left, f32 right, f32 bottom, f32 top, f32 zNear, f32 zFar)
    {
        Matrix4D outMatrix;
        f32* matrix = outMatrix.getPtr();

        matrix[0] = 2.f / (right - left);
        matrix[1] = 0.f;
        matrix[2] = 0.f;
        matrix[3] = 0.f;

        matrix[4] = 0.f;
        matrix[5] = 2.f / (top - bottom);
        matrix[6] = 0.f;
        matrix[7] = 0.f;

        matrix[8] = 0.f;
        matrix[9] = 0.f;
#if REVERSED_Z
        matrix[10] = 1.f / (zNear - zFar);
#else
        matrix[10] = 1.f / (zFar - zNear);
#endif //REVERSED_Z
        matrix[11] = 0.f;

        matrix[12] = -(right + left) / (right - left) /*0.f*/;
        matrix[13] = -(top + bottom) / (top - bottom) /*0.f*/;
#if REVERSED_Z
        matrix[14] = -zFar / (zNear - zFar);
#else
        matrix[14] = -zNear / (zFar - zNear);
#endif //REVERSED_Z
        matrix[15] = 1.f;

        return outMatrix;
    }

    inline Matrix4D buildLookAtMatrix(const Vector3D& position, const Vector3D& target, const Vector3D& upVector)
    {
        Vector3D forward = target - position;
        forward.normalize();

        Vector3D right = crossProduct(upVector, forward);
        right.normalize();

        Vector3D up = crossProduct(forward, right);

        Vector3D pos;
        pos.m_x = -dotProduct(right, position);
        pos.m_y = -dotProduct(up, position);
        pos.m_z = -dotProduct(forward, position);

        Matrix4D outMatrix(Vector4D(right, pos.m_x), Vector4D(up, pos.m_y), Vector4D(forward, pos.m_z), Vector4D(Vector3D(0.f), 1.f));
        outMatrix.makeTransposed();

        return outMatrix;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace math
} //namespace v3d

#include "Matrix4D.inl"
