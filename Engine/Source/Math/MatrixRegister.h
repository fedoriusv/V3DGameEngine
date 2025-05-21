#pragma once

#include "Math.h"
#include "VectorRegister.h"
#include "QuaternionRegister.h"
#include "DirectXMath.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////

/**
* @brief Reserved Z
* @see https://developer.nvidia.com/content/depth-precision-visualized
*/
#define ENABLE_REVERSE_Z 1

/////////////////////////////////////////////////////////////////////////////////////////////////////

namespace v3d
{
namespace math
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<u32 Rows, u32 Cols>
    concept ValidMatrixDim = (Rows == 2 && Cols == 2) || (Rows == 3 && Cols == 3) || (Rows == 4 && Cols == 4);

    /**
    * @brief TMatrixRegister class
    */
    template <RegisterType T, u32 Rows, u32 Cols> requires ValidMatrixDim<Rows, Cols>
    class TMatrixRegister
    {
    public:

        TMatrixRegister() noexcept;
        TMatrixRegister(const TMatrixRegister& other) noexcept;
        TMatrixRegister(const T* matrix) noexcept;

        TMatrixRegister(
            T m0, T m1, T m2,
            T m3, T m4, T m5,
            T m6, T m7, T m8) noexcept;

        TMatrixRegister(
            T m0, T m1, T m2, T m3,
            T m4, T m5, T m6, T m7,
            T m8, T m9, T m10, T m11,
            T m12, T m13, T m14, T m15) noexcept;

        ~TMatrixRegister() = default;

        T operator()(u32 row, u32 col) const;
        T operator[](u32 index) const;

        TMatrixRegister& operator=(const TMatrixRegister& other);

        [[nodiscard]] TMatrixRegister operator*(const TMatrixRegister& other) const;
        TMatrixRegister& operator*=(const TMatrixRegister& other);
        [[nodiscard]] TVectorRegister<T, Rows> operator*(const TVectorRegister<T, Rows>& vector) const;

        bool operator==(const TMatrixRegister& other) const;
        bool operator!=(const TMatrixRegister& other) const;

        void set(const T* matrix);
        void get(T* out) const;

        void makeIdentity();
        [[nodiscard]] bool isIdentity() const;

        void makeTransposed();
        [[nodiscard]] TMatrixRegister getTransposed() const;

        void setTranslation(const TVectorRegister<T, 3>& translation);
        [[nodiscard]] TVectorRegister<T, 3> getTranslation() const;

        void setRotation(const TVectorRegister<T, 3>& rotation);
        TVectorRegister<T, 3> getRotation() const;

        void setRotationQuaternion(const TQuaternionRegister<T>& rotation);
        [[nodiscard]] TQuaternionRegister<T> getRotationQuaternion() const;

        void setScale(const TVectorRegister<T, 3>& scale);
        [[nodiscard]] TVectorRegister<T, 3> getScale() const;

        void makeInverse();
        [[nodiscard]] TMatrixRegister getInversed() const;

        [[nodiscard]] T getDeterminant() const;

    private:

        DirectX::XMMATRIX _m;

        friend struct SVector;
        friend struct SMatrix;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    using MatrixRegister3x3 = TMatrixRegister<f32, 3, 3>;
    using MatrixRegister4x4 = TMatrixRegister<f32, 4, 4>;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    using Matrix4D = MatrixRegister4x4;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    struct SMatrix
    {
        template <FloatType T>
        static [[nodiscard]] MatrixRegister4x4 projectionMatrixPerspective(T fieldOfView, T aspectRatio, T zNear, T zFar);

        template <FloatType T>
        static [[nodiscard]] MatrixRegister4x4 projectionMatrixFrustum(T left, T right, T bottom, T top, T zNear, T zFar);

        template <FloatType T>
        static [[nodiscard]] MatrixRegister4x4 projectionMatrixOrtho(T left, T right, T bottom, T top, T zNear, T zFar);

        template <FloatType T>
        static [[nodiscard]] MatrixRegister4x4 lookAtMatrix(const TVectorRegister<T, 3>& position, const TVectorRegister<T, 3>& target, const TVectorRegister<T, 3>& upVector);
    };
 
    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace math
} //namespace v3d

#include "MatrixRegister.inl"
