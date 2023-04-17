#pragma once

#include "Math.h"
#include "Vector4D.h"
#include "Matrix3D.h"
#include "Matrix4D.h"

namespace v3d
{
namespace math
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief TTQuaternion class
    */
    template<class T>
    class TQuaternion
    {
    public:

        TQuaternion() noexcept;
        TQuaternion(const TQuaternion& other) noexcept;
        TQuaternion(const TVector4D<T>& vector) noexcept;
        TQuaternion(const T nx, const T ny, const T nz, const T nw) noexcept;
        ~TQuaternion() = default;

        TQuaternion<T>& operator=(const TQuaternion<T>& other) const;

        [[nodiscard]] T operator[](u32 index) const;
        T& operator[](u32 index);

        bool operator==(const TQuaternion<T>& other) const;
        bool operator!=(const TQuaternion<T>& other) const;

        [[nodiscard]] TQuaternion<T> operator+(const TQuaternion<T>& other) const;
        [[nodiscard]] TQuaternion<T> operator+(const T scalar) const;
        TQuaternion<T>& operator+=(const TQuaternion<T>& other);
        TQuaternion<T>& operator+=(const T scalar);
        [[nodiscard]] TQuaternion<T> operator-(const TQuaternion<T>& other) const;
        [[nodiscard]] TQuaternion<T> operator-(const T scalar) const;
        TQuaternion<T>& operator-=(const TQuaternion<T>& other);
        TQuaternion<T>& operator-=(const T scalar);
        [[nodiscard]] TQuaternion<T> operator*(const TQuaternion<T>& other) const;
        [[nodiscard]] TQuaternion<T> operator*(const T scalar) const;
        TQuaternion<T>& operator*=(const TQuaternion<T>& other);
        TQuaternion<T>& operator*=(const T scalar);
        [[nodiscard]] TQuaternion<T> operator/(const T scalar) const;
        TQuaternion<T>& operator/=(const T scalar);

        [[nodiscard]] TQuaternion<T> operator-() const;

        TQuaternion<T>& set(const T nx, const T ny, const T nz, const T nw);
        TQuaternion<T>& set(const  TVector4D<T>& vector);

        void makeIdentity();
        [[nodiscard]] bool isIdentity() const;

        TQuaternion<T>& normalize();
        TQuaternion<T>& invert();

        T m_x;
        T m_y;
        T m_z;
        T m_w;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<class T>
    [[nodiscard]] inline T dotProduct(const TQuaternion<T>& q1, const TQuaternion<T>& q2)
    {
        return (q1.m_x * q2.m_x + q1.m_y * q2.m_y + q1.m_z * q2.m_z + q1.m_w * q2.m_w);
    }

    template<class T>
    [[nodiscard]] TQuaternion<T> lerp(const TQuaternion<T>& q1, const TQuaternion<T>& q2, f32 t)
    {
        TQuaternion<T> temp;
        if (dotProduct<T>(q1, q2) < (T)0)
        {
            temp.set(q1.m_x + t * (-q2.m_x - q1.m_x),
                q1.m_y + t * (-q2.m_y - q1.m_y),
                q1.m_z + t * (-q2.m_z - q1.m_z),
                q1.m_w + t * (-q2.m_w - q1.m_w));
        }
        else
        {
            temp.set(q1.m_x + t * (q2.m_x - q1.m_x),
                q1.m_y + t * (q2.m_y - q1.m_y),
                q1.m_z + t * (q2.m_z - q1.m_z),
                q1.m_w + t * (q2.m_w - q1.m_w));
        }
        return temp.normalize();
    }

    template<class T>
    [[nodiscard]] TQuaternion<T> slerp(const TQuaternion<T>& q1, const TQuaternion<T>& q2, f32 t)
    {
        T dot = dotProduct<T>(q1, q2);

        TQuaternion<T> temp;
        if (dot < (T)0)
        {
            dot = -dot;
            temp.set(-q2.m_x, -q2.m_y, -q2.m_z, -q2.m_w);
        }
        else
        {
            temp = q2;
        }

        if (dot < (T)(0.95))
        {
            T angle = std::acos<T>(dot);
            T sinadiv = (T)1 / std::sin<T>(angle);
            T sinat = std::sin<T>(angle * t);
            T sinaomt = std::sin<T>(angle * ((T)1 - t));

            temp.set((q1.m_x * sinaomt + temp.m_x * sinat) * sinadiv,
                (q1.m_y * sinaomt + temp.m_y * sinat) * sinadiv,
                (q1.m_z * sinaomt + temp.m_z * sinat) * sinadiv,
                (q1.m_w * sinaomt + temp.m_w * sinat) * sinadiv);

            return temp;
        }
        else // small angle (0.05)
        {
            return lerp(q1, temp, t);
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<class T>
    TVector4D<T> quaternionToEuler(const TQuaternion<T>& quat)
    {
        //TODO
        return TVector4D<T>();
    }

    template<class T>
    TQuaternion<T> eulerToQuaternion(const TVector4D<T>& euler)
    {
        //TODO
        return TQuaternion<T>();
    }

    template<class T>
    TMatrix3D<T> quaternionToMatrix(const TQuaternion<T>& q)
    {
        //TODO
        return TMatrix3D<T>();
    }
    
    template<class T>
    TQuaternion<T> matrixToQuaternion(const TMatrix3D<T>& m)
    {
        //TODO
        return TQuaternion<T>();
    }

    template<class T>
    TMatrix4D<T> quaternionToMatrix(const TQuaternion<T>& q)
    {
        //TODO
        return TMatrix4D<T>();
    }
    
    template<class T>
    TQuaternion<T> matrixToQuaternion(const TMatrix4D<T>& m)
    {
        //TODO
        return TQuaternion<T>();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace math
} //namespace v3d

#include "Quaternion.inl"