#pragma once

#include "Math.h"
#include "DirectXMath.h"

namespace v3d
{
namespace math
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<typename T>
    concept RegisterType = std::is_same_v<T, u32> || std::is_same_v<T, f32>;

    template<u32 Dim>
    concept ValidVectorDim = (Dim >= 2 && Dim <= 4);

    /**
    * @brief TVectorRegister struct
    */
    template<RegisterType T, u32 Dim> requires ValidVectorDim<Dim>
    class alignas(16) TVectorRegister final
    {
    public:

        TVectorRegister() noexcept;
        TVectorRegister(T x, T y) noexcept;
        TVectorRegister(T x, T y, T z) noexcept;
        TVectorRegister(T x, T y, T z, T w) noexcept;
        TVectorRegister(const TVectorRegister& other) noexcept;
        ~TVectorRegister() noexcept = default;

        TVectorRegister& operator=(const TVectorRegister& other);

        [[nodiscard]] TVectorRegister operator+(const TVectorRegister& other) const;
        [[nodiscard]] TVectorRegister operator+(T scalar) const;
        TVectorRegister& operator+=(const TVectorRegister& other);
        TVectorRegister& operator+=(T scalar);
        [[nodiscard]] TVectorRegister operator-(const TVectorRegister& other) const;
        [[nodiscard]] TVectorRegister operator-(T scalar) const;
        TVectorRegister& operator-=(const TVectorRegister& other);
        TVectorRegister& operator-=(T scalar);
        [[nodiscard]] TVectorRegister operator*(const TVectorRegister& other) const;
        [[nodiscard]] TVectorRegister operator*(T scalar) const;
        TVectorRegister& operator*=(const TVectorRegister& other);
        TVectorRegister& operator*=(T scalar);
        [[nodiscard]] TVectorRegister operator/(const TVectorRegister& other) const;
        [[nodiscard]] TVectorRegister operator/(T scalar) const;
        TVectorRegister& operator/=(const TVectorRegister& other);
        TVectorRegister& operator/=(T scalar);
        [[nodiscard]] TVectorRegister operator-() const;

        bool operator<=(const TVectorRegister&other) const;
        bool operator>=(const TVectorRegister&other) const;
        bool operator<(const TVectorRegister&other) const;
        bool operator>(const TVectorRegister&other) const;
        bool operator==(const TVectorRegister& other) const;
        bool operator!=(const TVectorRegister& other) const;

        [[nodiscard]] T operator[](u32 index) const;

        T getX() const;
        T getY() const;
        T getZ() const;
        T getW() const;

        void setX(T x);
        void setY(T y);
        void setZ(T z);
        void setW(T w);

        TVectorRegister& set(T x, T y);
        TVectorRegister& set(T x, T y, T z);
        TVectorRegister& set(T x, T y, T z, T w);
        TVectorRegister& set(const TVectorRegister& other);

        [[nodiscard]] bool isEquals(const TVectorRegister& other, f32 tolerance = k_tolerance32) const;

        [[nodiscard]] T length() const;
        [[nodiscard]] T lengthSQ() const;

        [[nodiscard]] T distanceFrom(const TVectorRegister& other) const;
        [[nodiscard]] T distanceFromSQ(const TVectorRegister& other) const;

        TVectorRegister& normalize();

    private:

        DirectX::XMVECTOR _v;

        template <RegisterType T, u32 Rows, u32 Cols> requires ValidMatrixDim<Rows, Cols>
        friend class TMatrixRegister;
        friend struct SVector;
        friend struct SMatrix;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    using VectorRegister2D = TVectorRegister<f32, 2>;
    using VectorRegister3D = TVectorRegister<f32, 3>;
    using VectorRegister4D = TVectorRegister<f32, 4>;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    using Vector2D = VectorRegister2D;
    using Vector3D = VectorRegister3D;
    using Vector4D = VectorRegister4D;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    struct SVector
    {
        template<RegisterType T, u32 Dim>
        [[nodiscard]] static T distance(const TVectorRegister<T, Dim>& v1, const TVectorRegister<T, Dim>& v2);

        template<RegisterType T, u32 Dim>
        [[nodiscard]] static T dot(const TVectorRegister<T, Dim>& v1, const TVectorRegister<T, Dim>& v2);

        template<RegisterType T, u32 Dim>
        [[nodiscard]] static TVectorRegister<T, Dim> cross(const TVectorRegister<T, Dim>& v1, const TVectorRegister<T, Dim>& v2);

        template<RegisterType T, u32 Dim>
        [[nodiscard]] static TVectorRegister<T, Dim> lerp(const TVectorRegister<T, Dim>& v1, const TVectorRegister<T, Dim>& v2, T d);
    };

    template<typename S, RegisterType T, u32 Dim>
    [[nodiscard]] static TVectorRegister<T, Dim> operator*(S scalar, const TVectorRegister<T, Dim>& vector);

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace math
} //namespace v3d

#include "VectorRegister.inl"