#pragma once

#include "Vector3D.h"
#include "Vector4D.h"

#define DEPTH_ZERO_TO_ONE 1 //[0..1]

namespace v3d
{
namespace core
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template <class T>
    class TMatrix4D
    {
    public:

        TMatrix4D();
        TMatrix4D(const TMatrix4D<T>& other);
        TMatrix4D(const TVector4D<T>& col0, const TVector4D<T>& col1, const TVector4D<T>& col2, const TVector4D<T>& col3);
        TMatrix4D(const T* matrix);
        TMatrix4D(const T& m0, const T& m1, const T& m2, const T& m3,
            const T& m4, const T& m5, const T& m6, const T& m7,
            const T& m8, const T& m9, const T& m10, const T& m11,
            const T& m12, const T& m13, const T& m14, const T& m15);

        T&              operator()(u32 row, u32 col);
        const T&        operator()(u32 row, u32 col)            const;
        T&              operator[](u32 index);
        const T&        operator[](u32 index)                   const;

        TMatrix4D<T>&   operator=(const T& scalar);
        bool            operator==(const TMatrix4D<T>& other)   const;
        bool            operator!=(const TMatrix4D<T>& other)   const;
        TMatrix4D<T>    operator+(const TMatrix4D<T>& other)    const;
        TMatrix4D<T>&   operator+=(const TMatrix4D<T>& other);
        TMatrix4D<T>    operator-(const TMatrix4D<T>& other)    const;
        TMatrix4D<T>&   operator-=(const TMatrix4D<T>& other);
        TMatrix4D<T>    operator*(const TMatrix4D<T>& other)    const;
        TMatrix4D<T>&   operator*=(const TMatrix4D<T>& other);
        TMatrix4D<T>    operator*(const T& scalar)              const;
        TMatrix4D<T>&   operator*=(const T& scalar);
        TVector4D<T>    operator*(const TVector4D<T>& vector)   const;

        void            set(const T* matrix);

        T*              getPtr();
        const T*        getPtr()                                const;

        void            makeIdentity();
        bool            isIdentity()                            const;

        void            makeTransposed();
        TMatrix4D<T>    getTransposed()                         const;

        void            setTranslation(const TVector3D<T>& translation);
        TVector3D<T>    getTranslation()                        const;

        void            setRotation(const TVector3D<T>& rotation);
        TVector3D<T>    getRotation()                           const;

        void            preScale(const TVector3D<T>& scale);
        void            postScale(const TVector3D<T>& scale);
        void            setScale(const TVector3D<T>& scale);

        TVector3D<T>    getScale()                              const;

        bool            makeInverse();
        bool            getInverse(TMatrix4D<T>& outMatrix)     const;

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

    using Matrix4D   = TMatrix4D<f32>;
    using Matrix4D64 = TMatrix4D<f64>;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    inline Matrix4D buildProjectionMatrixPerspective(f32 fieldOfViewRadians, f32 aspectRatio, f32 zNear, f32 zFar)
    {
        Matrix4D outMatrix;

        f32* matrix = outMatrix.getPtr();

        const f32 yFac = tanf(fieldOfViewRadians / 2.0f);
        const f32 xFac = yFac * aspectRatio;

        matrix[0] = 1 / xFac;
        matrix[1] = 0;
        matrix[2] = 0;
        matrix[3] = 0;

        matrix[4] = 0;
        matrix[5] = 1 / yFac;
        matrix[6] = 0;
        matrix[7] = 0;

        matrix[8] = 0;
        matrix[9] = 0;
#if DEPTH_ZERO_TO_ONE
        matrix[10] = zFar / (zNear - zFar);
#else
        matrix[10] = - (zFar + zNear) / (zFar - zNear);
#endif //DEPTH_ZERO_TO_ONE
        matrix[11] = -1;

        matrix[12] = 0;
        matrix[13] = 0;
#if DEPTH_ZERO_TO_ONE
        matrix[14] = - (zFar * zNear) / (zFar - zNear);
#else
        matrix[14] = - (2 * zFar * zNear) / (zFar - zNear);
#endif //DEPTH_ZERO_TO_ONE
        matrix[15] = 0;

        return outMatrix;
    }

    inline Matrix4D buildProjectionMatrixOrtho(f32 left, f32 right, f32 top, f32 bottom, f32 zNear, f32 zFar)
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
#if DEPTH_ZERO_TO_ONE
        matrix[10] = 1.f / (zFar - zNear);
#else
        matrix[10] = 2.f / (zFar - zNear);
#endif //DEPTH_ZERO_TO_ONE
        matrix[11] = 0.f;

        matrix[12] = - (right + left) / (right - left) /*0.f*/;
        matrix[13] = - (top + bottom) / (top - bottom) /*0.f*/;
#if DEPTH_ZERO_TO_ONE
        matrix[14] = - zNear / (zFar - zNear);
#else	
        matrix[14] = - (zFar + zNear) / (zFar - zNear);
#endif //DEPTH_ZERO_TO_ONE
        matrix[15] = 1.f;

        return outMatrix;
    }

    inline Matrix4D buildLookAtMatrix(const Vector3D& position, const Vector3D& target, const Vector3D& upVector)
    {
        Vector3D v3X, v3Y, v3Z;
        v3Y = upVector;
        v3Y.normalize();

        v3Z = position - target;
        v3Z.normalize();

        v3X = crossProduct(v3Y, v3Z);
        v3X.normalize();

        v3Y = crossProduct(v3Z, v3X);

        Matrix4D outMatrix;
        outMatrix = Matrix4D(Vector4D(v3X), Vector4D(v3Y), Vector4D(v3Z), Vector4D(position, 1.0f));
        outMatrix.makeInverse();

        return outMatrix;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace core
} //namespace v3d

#include "Matrix4D.inl"
