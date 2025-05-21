#pragma once

#include "Math.h"
#include "VectorRegister.h"

namespace v3d
{
namespace math
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief TQuaternionRegister class
    */
    template<RegisterType T>
    class alignas(16) TQuaternionRegister
    {
    public:

        TQuaternionRegister() noexcept;
        TQuaternionRegister(const TQuaternionRegister& other) noexcept;
        TQuaternionRegister(const TVectorRegister<T, 3>& vector) noexcept;
        TQuaternionRegister(const TVectorRegister<T, 4>& vector) noexcept;
        TQuaternionRegister(T x, T y, T z) noexcept;
        TQuaternionRegister(T x, T y, T z, T w) noexcept;
        ~TQuaternionRegister() = default;

        TQuaternionRegister<T>& operator=(const TQuaternionRegister<T>& other) const;

        [[nodiscard]] TQuaternionRegister<T> operator*(const TQuaternionRegister<T>& other) const;
        TQuaternionRegister<T>& operator*=(const TQuaternionRegister<T>& other);

        bool operator==(const TQuaternionRegister<T>& other) const;
        bool operator!=(const TQuaternionRegister<T>& other) const;

        [[nodiscard]] T operator[](u32 index) const;

        T getX() const;
        T getY() const;
        T getZ() const;
        T getW() const;

        TQuaternionRegister<T>& set(T x, T y, T z, T w);
        TQuaternionRegister<T>& set(const TVectorRegister<T, 4>& vector);

        [[nodiscard]] T length() const;
        [[nodiscard]] T lengthSQ() const;

        void makeIdentity();
        [[nodiscard]] bool isIdentity() const;

        TQuaternionRegister<T>& normalize();
        TQuaternionRegister<T>& invert();

    private:

        DirectX::XMVECTOR _q;

        template <RegisterType T, u32 Rows, u32 Cols> requires ValidMatrixDim<Rows, Cols>
        friend class TMatrixRegister;
        friend struct SQuaternion;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    using QuaternionRegister = TQuaternionRegister<f32>;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    using Quaternion = QuaternionRegister;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    struct SQuaternion
    {
        template<RegisterType T>
        [[nodiscard]] T static dot(const TQuaternionRegister<T>& q1, const TQuaternionRegister<T>& q2);

        template<RegisterType T>
        [[nodiscard]] TQuaternionRegister<T> static lerp(const TQuaternionRegister<T>& q1, const TQuaternionRegister<T>& q2, T d);

        template<RegisterType T>
        [[nodiscard]] TQuaternionRegister<T> static slerp(const TQuaternionRegister<T>& q1, const TQuaternionRegister<T>& q2, T d);
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace math
} //namespace v3d

#include "QuaternionRegister.inl"