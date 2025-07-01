#include "DirectXMath.h"
#include "MatrixRegister.h"

namespace v3d
{
namespace math
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template <RegisterType T, u32 Rows, u32 Cols> requires ValidMatrixDim<Rows, Cols>
    TMatrixRegister<T, Rows, Cols>::TMatrixRegister() noexcept
    {
        makeIdentity();
    }

    template <RegisterType T, u32 Rows, u32 Cols> requires ValidMatrixDim<Rows, Cols>
    TMatrixRegister<T, Rows, Cols>::TMatrixRegister(const TMatrixRegister<T, Rows, Cols>& other) noexcept
        : _m(other._m)
    {
    }

    template <RegisterType T, u32 Rows, u32 Cols> requires ValidMatrixDim<Rows, Cols>
    TMatrixRegister<T, Rows, Cols>::TMatrixRegister(const T* matrix) noexcept
    {
        set(matrix);
    }

    template<RegisterType T, u32 Rows, u32 Cols> requires ValidMatrixDim<Rows, Cols>
    inline TMatrixRegister<T, Rows, Cols>::TMatrixRegister(
        T m0, T m1, T m2,
        T m3, T m4, T m5,
        T m6, T m7, T m8) noexcept
    {
        static_assert(Rows == 3 && Cols == 3, "Unsupported dimension");
        _m = DirectX::XMMatrixSet(
            m0, m1, m2, 0,
            m3, m4, m5, 0,
            m6, m7, m8, 0,
            0, 0, 0, 1);
    }

    template <RegisterType T, u32 Rows, u32 Cols> requires ValidMatrixDim<Rows, Cols>
    TMatrixRegister<T, Rows, Cols>::TMatrixRegister(
        T m0, T m1, T m2, T m3,
        T m4, T m5, T m6, T m7,
        T m8, T m9, T m10, T m11,
        T m12, T m13, T m14, T m15) noexcept
    {
        static_assert(Rows == 4 && Cols == 4, "Unsupported dimension");
        _m = DirectX::XMMatrixSet(
            m0, m1, m2, m3,
            m4, m5, m6, m7,
            m8, m9, m10, m11,
            m12, m13, m14, m15);
    }

    template <RegisterType T, u32 Rows, u32 Cols> requires ValidMatrixDim<Rows, Cols>
    inline T TMatrixRegister<T, Rows, Cols>::operator()(u32 row, u32 col) const
    {
        DirectX::XMVECTOR r = _m.r[row];
        return DirectX::XMVectorGetByIndex(r, col);
    }

    template <RegisterType T, u32 Rows, u32 Cols> requires ValidMatrixDim<Rows, Cols>
    inline T TMatrixRegister<T, Rows, Cols>::operator[](u32 index) const
    {
        u32 row = index / Rows;
        u32 col = index % Cols;
        DirectX::XMVECTOR r = _m.r[row];
        return DirectX::XMVectorGetByIndex(r, col);
    }

    template <RegisterType T, u32 Rows, u32 Cols> requires ValidMatrixDim<Rows, Cols>
    inline TMatrixRegister<T, Rows, Cols>& TMatrixRegister<T, Rows, Cols>::operator*=(const TMatrixRegister<T, Rows, Cols>& other)
    {
        TMatrixRegister<T, Rows, Cols> m;
        _m = DirectX::XMMatrixMultiply(_m, other._m);
        return *this;
    }


    template <RegisterType T, u32 Rows, u32 Cols> requires ValidMatrixDim<Rows, Cols>
    inline TMatrixRegister<T, Rows, Cols> TMatrixRegister<T, Rows, Cols>::operator*(const TMatrixRegister<T, Rows, Cols>& other) const
    {
        TMatrixRegister<T, Rows, Cols> m;
        m._m = DirectX::XMMatrixMultiply(_m, other._m);
        return m;
    }

    template<RegisterType T, u32 Rows, u32 Cols> requires ValidMatrixDim<Rows, Cols>
    inline TVectorRegister<T, Rows> TMatrixRegister<T, Rows, Cols>::operator*(const TVectorRegister<T, Rows>& vector) const
    {
        TVectorRegister<T, Rows> v;
        if constexpr (Rows == 2)
        {
            v._v = DirectX::XMVector2Transform(vector._v, _m);
        }
        else if constexpr (Rows == 3)
        {
            v._v = DirectX::XMVector3Transform(vector._v, _m);
        }
        else if constexpr (Rows == 4)
        {
            v._v = DirectX::XMVector4Transform(vector._v, _m);
        }

        return v;
    }

    template <RegisterType T, u32 Rows, u32 Cols> requires ValidMatrixDim<Rows, Cols>
    inline void TMatrixRegister<T, Rows, Cols>::makeIdentity()
    {
        _m = DirectX::XMMatrixIdentity();
    }

    template <RegisterType T, u32 Rows, u32 Cols> requires ValidMatrixDim<Rows, Cols>
    inline bool TMatrixRegister<T, Rows, Cols>::isIdentity() const
    {
        return DirectX::XMMatrixIsIdentity(_m);
    }

    template<RegisterType T, u32 Rows, u32 Cols> requires ValidMatrixDim<Rows, Cols>
    inline void TMatrixRegister<T, Rows, Cols>::setScale(const TVectorRegister<T, 3>& scale)
    {
        static_assert(Rows == 4 && Cols == 4, "Unsupported dimension");
        DirectX::XMVECTOR oldScale;
        DirectX::XMVECTOR rotationQuat;
        DirectX::XMVECTOR translation;

        if (DirectX::XMMatrixDecompose(&oldScale, &rotationQuat, &translation, _m))
        {
            DirectX::XMVECTOR newScale = DirectX::XMVectorSet(DirectX::XMVectorGetX(scale._v), DirectX::XMVectorGetY(scale._v), DirectX::XMVectorGetZ(scale._v), 0.0f);

            DirectX::XMMATRIX scaleMatrix = DirectX::XMMatrixScalingFromVector(newScale);
            DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationQuaternion(rotationQuat);
            DirectX::XMMATRIX translationMatrix = DirectX::XMMatrixTranslationFromVector(translation);

            _m = scaleMatrix * rotationMatrix * translationMatrix;
        }
    }

    template<RegisterType T, u32 Rows, u32 Cols> requires ValidMatrixDim<Rows, Cols>
    inline TVectorRegister<T, 3> TMatrixRegister<T, Rows, Cols>::getScale() const
    {
        static_assert(Rows == 4 && Cols == 4, "Unsupported dimension");
        DirectX::XMVECTOR scale;
        [[maybe_unused]] DirectX::XMVECTOR rotationQuat;
        [[maybe_unused]] DirectX::XMVECTOR translation;
        if (XMMatrixDecompose(&scale, &rotationQuat, &translation, _m))
        {
            TVectorRegister<T, 3> v;
            v.set(DirectX::XMVectorGetX(scale), DirectX::XMVectorGetY(scale), DirectX::XMVectorGetZ(scale));
            return v;
        }

        // Decomposition failed
        return  TVectorRegister<T, 3>((T)1, (T)1, (T)1);
    }

    template <RegisterType T, u32 Rows, u32 Cols> requires ValidMatrixDim<Rows, Cols>
    inline void TMatrixRegister<T, Rows, Cols>::makeInverse()
    {
        _m = DirectX::XMMatrixInverse(nullptr, _m);
    }

    template<RegisterType T, u32 Rows, u32 Cols> requires ValidMatrixDim<Rows, Cols>
    inline TMatrixRegister<T, Rows, Cols> TMatrixRegister<T, Rows, Cols>::getInversed() const
    {
        TMatrixRegister<T, Rows, Cols> m;
        m._m = DirectX::XMMatrixInverse(nullptr, _m);
        return m;
    }

    template<RegisterType T, u32 Rows, u32 Cols> requires ValidMatrixDim<Rows, Cols>
    inline T TMatrixRegister<T, Rows, Cols>::getDeterminant() const
    {
        return DirectX::XMVectorGetX(DirectX::XMMatrixDeterminant(_m));
    }

    template <RegisterType T, u32 Rows, u32 Cols> requires ValidMatrixDim<Rows, Cols>
    inline TMatrixRegister<T, Rows, Cols>& TMatrixRegister<T, Rows, Cols>::operator=(const TMatrixRegister<T, Rows, Cols>& other)
    {
        if (this == &other)
        {
            return *this;
        }

        _m = other._m;
        return *this;
    }

    template <RegisterType T, u32 Rows, u32 Cols> requires ValidMatrixDim<Rows, Cols>
    inline bool TMatrixRegister<T, Rows, Cols>::operator==(const TMatrixRegister<T, Rows, Cols> &other) const
    {
        for (s32 i = 0; i < Rows; ++i)
        {
            if constexpr (Cols == 2)
            {
                if (!DirectX::XMVector2Equal(_m[i], other._m[i]))
                {
                    return false;
                }
            }
            else if constexpr (Cols == 3)
            {
                if (!DirectX::XMVector3Equal(_m[i], other._m[i]))
                {
                    return false;
                }
            }
            else if constexpr (Cols == 4)
            {
                if (!DirectX::XMVector4Equal(_m[i], other._m[i]))
                {
                    return false;
                }
            }
        }
        return true;
    }

    template <RegisterType T, u32 Rows, u32 Cols> requires ValidMatrixDim<Rows, Cols>
    inline bool TMatrixRegister<T, Rows, Cols>::operator!=(const TMatrixRegister<T, Rows, Cols> &other) const
    {
        for (s32 i = 0; i < Rows; ++i)
        {
            if constexpr (Cols == 2)
            {
                if (!DirectX::XMVector2NotEqual(_m[i], other._m[i]))
                {
                    return false;
                }
            }
            else if constexpr (Cols == 3)
            {
                if (!DirectX::XMVector2NotEqual(_m[i], other._m[i]))
                {
                    return false;
                }
            }
            else if constexpr (Cols == 4)
            {
                if (!DirectX::XMVector2NotEqual(_m[i], other._m[i]))
                {
                    return false;
                }
            }
        }
        return true;
    }

    template <RegisterType T, u32 Rows, u32 Cols> requires ValidMatrixDim<Rows, Cols>
    inline void TMatrixRegister<T, Rows, Cols>::makeTransposed()
    {
        _m = DirectX::XMMatrixTranspose(_m);
    }

    template <RegisterType T, u32 Rows, u32 Cols> requires ValidMatrixDim<Rows, Cols>
    inline TMatrixRegister<T, Rows, Cols> TMatrixRegister<T, Rows, Cols>::getTransposed() const
    {
        TMatrixRegister<T, Rows, Cols> v;
        v._m = DirectX::XMMatrixTranspose(_m);
        return v;
    }

    template<RegisterType T, u32 Rows, u32 Cols> requires ValidMatrixDim<Rows, Cols>
    inline void TMatrixRegister<T, Rows, Cols>::setTranslation(const TVectorRegister<T, 3>& translation)
    {
        static_assert(Rows == 4 && Cols == 4, "Unsupported dimension");
        _m.r[3] = DirectX::XMVectorSet(DirectX::XMVectorGetX(translation._v), DirectX::XMVectorGetY(translation._v), DirectX::XMVectorGetZ(translation._v), 1);
    }

    template<RegisterType T, u32 Rows, u32 Cols> requires ValidMatrixDim<Rows, Cols>
    inline TVectorRegister<T, 3> TMatrixRegister<T, Rows, Cols>::getTranslation() const
    {
        static_assert(Rows == 4 && Cols == 4, "Unsupported dimension");
        TVectorRegister<T, 3> v;
        v._v = DirectX::XMVectorSet(DirectX::XMVectorGetX(_m.r[3]), DirectX::XMVectorGetY(_m.r[3]), DirectX::XMVectorGetZ(_m.r[3]), 0);
        return v;
    }

    template<RegisterType T, u32 Rows, u32 Cols> requires ValidMatrixDim<Rows, Cols>
    inline void TMatrixRegister<T, Rows, Cols>::setRotation(const TVectorRegister<T, 3>& rotation)
    {
        static_assert(Rows == 4 && Cols == 4, "Unsupported dimension");
        DirectX::XMVECTOR scale;
        [[maybe_unused]] DirectX::XMVECTOR rotationQuat;
        DirectX::XMVECTOR translation;
        if (DirectX::XMMatrixDecompose(&scale, &rotationQuat, &translation, _m))
        {
            DirectX::XMVECTOR v = DirectX::XMVectorSet(DirectX::XMVectorGetX(rotation._v) * k_degToRad, DirectX::XMVectorGetY(rotation._v) * k_degToRad, DirectX::XMVectorGetZ(rotation._v) * k_degToRad, 1);

            DirectX::XMMATRIX scaleMatrix = DirectX::XMMatrixScalingFromVector(scale);
            DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationRollPitchYawFromVector(v);
            DirectX::XMMATRIX translationMatrix = DirectX::XMMatrixTranslationFromVector(translation);

            _m = scaleMatrix * rotationMatrix * translationMatrix;
        }
    }

    template<RegisterType T, u32 Rows, u32 Cols> requires ValidMatrixDim<Rows, Cols>
    inline TVectorRegister<T, 3> TMatrixRegister<T, Rows, Cols>::getRotation() const
    {
        static_assert(Rows == 4 && Cols == 4, "Unsupported dimension");
        [[maybe_unused]] DirectX::XMVECTOR scale;
        DirectX::XMVECTOR rotationQuat;
        [[maybe_unused]] DirectX::XMVECTOR translation;
        if (DirectX::XMMatrixDecompose(&scale, &rotationQuat, &translation, _m))
        {
            DirectX::XMMATRIX rotMatrix = DirectX::XMMatrixRotationQuaternion(rotationQuat);

            // Extract Euler angles (pitch, yaw, roll) from rotation matrix
            f32 pitch = std::asinf(-rotMatrix.r[2].m128_f32[1]); // Y-axis of forward
            f32 yaw = std::atan2f(rotMatrix.r[2].m128_f32[0], rotMatrix.r[2].m128_f32[2]);
            f32 roll = std::atan2f(rotMatrix.r[0].m128_f32[1], rotMatrix.r[1].m128_f32[1]);

            return TVectorRegister<T, 3>(pitch * k_radToDeg, yaw * k_radToDeg, roll * k_radToDeg);
        }

        return TVectorRegister<T, 3>();
    }

    template<RegisterType T, u32 Rows, u32 Cols> requires ValidMatrixDim<Rows, Cols>
    inline void TMatrixRegister<T, Rows, Cols>::setRotationQuaternion(const TQuaternionRegister<T>& rotation)
    {
        static_assert(Rows == 4 && Cols == 4, "Unsupported dimension");
        DirectX::XMMATRIX rotMatrix = DirectX::XMMatrixRotationQuaternion(rotation);
        _m = DirectX::XMMatrixMultiply(_m, rotMatrix);
    }

    template<RegisterType T, u32 Rows, u32 Cols> requires ValidMatrixDim<Rows, Cols>
    inline TQuaternionRegister<T> TMatrixRegister<T, Rows, Cols>::getRotationQuaternion() const
    {
        static_assert(Rows == 4 && Cols == 4, "Unsupported dimension");
        DirectX::XMMATRIX rotationOnly = _m;
        rotationOnly.r[3] = DirectX::XMVectorSet(0, 0, 0, 1); // Zero translation

        TQuaternionRegister<T> q;
        q._q = DirectX::XMQuaternionRotationMatrix(rotationOnly);
        return q;
    }

    template <RegisterType T, u32 Rows, u32 Cols> requires ValidMatrixDim<Rows, Cols>
    inline void TMatrixRegister<T, Rows, Cols>::set(const T* matrix)
    {
        _m = DirectX::XMMatrixSet(
            matrix[0], matrix[1], matrix[2], matrix[3],
            matrix[4], matrix[5], matrix[6], matrix[7],
            matrix[8], matrix[9], matrix[10], matrix[11],
            matrix[12], matrix[13], matrix[14], matrix[15]);
    }

    template<RegisterType T, u32 Rows, u32 Cols> requires ValidMatrixDim<Rows, Cols>
    inline void TMatrixRegister<T, Rows, Cols>::get(T* out) const
    {
        if constexpr (Rows == 4 && Cols == 4)
        {
            DirectX::XMStoreFloat4x4((DirectX::XMFLOAT4X4*)out, _m);
        }
        else if (Rows == 3 && Cols == 3)
        {
            DirectX::XMStoreFloat3x3((DirectX::XMFLOAT3X3*)out, _m);
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<FloatType T>
    inline MatrixRegister4x4 SMatrix::projectionMatrixPerspective(T fieldOfView, T aspectRatio, T zNear, T zFar)
    {
        MatrixRegister4x4 m;
#if ENABLE_REVERSED_Z
        m._m = DirectX::XMMatrixPerspectiveFovLH(fieldOfView, aspectRatio, zFar, zNear);
#else
        m._m = DirectX::XMMatrixPerspectiveFovLH(fieldOfView, aspectRatio, zNear, zFar);
#endif
        return m;
    }

    template<FloatType T>
    inline MatrixRegister4x4 SMatrix::projectionMatrixFrustum(T left, T right, T bottom, T top, T zNear, T zFar)
    {
        MatrixRegister4x4 m;
#if ENABLE_REVERSED_Z
        m._m = DirectX::XMMatrixPerspectiveLH(right - left, top - bottom, zFar, zNear);
#else
        m._m = DirectX::XMMatrixPerspectiveLH(right - left, top - bottom, zNear, zFar);
#endif
        return m;
    }

    template<FloatType T>
    inline MatrixRegister4x4 SMatrix::projectionMatrixOrtho(T left, T right, T bottom, T top, T zNear, T zFar)
    {
        MatrixRegister4x4 m;
#if ENABLE_REVERSED_Z
        m._m = DirectX::XMMatrixOrthographicLH(right - left, top - bottom, zFar, zNear);
#else
        m._m = DirectX::XMMatrixOrthographicLH(right - left, top - bottom, zNear, zFar);
#endif
        return m;
    }

    template<FloatType T>
    MatrixRegister4x4 SMatrix::lookAtMatrix(const TVectorRegister<T, 3>& position, const TVectorRegister<T, 3>& target, const TVectorRegister<T, 3>& upVector)
    {
        MatrixRegister4x4 m;
        m._m = DirectX::XMMatrixLookAtLH(position._v, target._v, upVector._v);
        return m;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace math
} //namespace v3d
