#include "DirectXMath.h"
#include "VectorRegister.h"

namespace v3d
{
namespace math
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<RegisterType T, u32 Dim> requires ValidVectorDim<Dim>
    inline TVectorRegister<T, Dim>::TVectorRegister() noexcept
        : _v(DirectX::XMVectorZero())
    {
    }

    template<RegisterType T, u32 Dim> requires ValidVectorDim<Dim>
    inline TVectorRegister<T, Dim>::TVectorRegister(T x, T y) noexcept
    {
        static_assert(Dim == 2, "Unsupported dimension");
        if constexpr (std::is_same_v<T, u32>)
        {
            _v = DirectX::XMVectorSetInt(x, y, 0, 0);
        }
        else
        {
            _v = DirectX::XMVectorSet(x, y, k_NaN, k_NaN);
        }
    }

    template<RegisterType T, u32 Dim> requires ValidVectorDim<Dim>
    inline TVectorRegister<T, Dim>::TVectorRegister(T x, T y, T z) noexcept
    {
        static_assert(Dim == 3, "Unsupported dimension");
        if constexpr (std::is_same_v<T, u32>)
        {
            _v = DirectX::XMVectorSetInt(x, y, z, 0);
        }
        else
        {
            _v = DirectX::XMVectorSet(x, y, z, k_NaN);
        }
    }

    template<RegisterType T, u32 Dim> requires ValidVectorDim<Dim>
    inline TVectorRegister<T, Dim>::TVectorRegister(T x, T y, T z, T w) noexcept
    {
        static_assert(Dim == 4, "Unsupported dimension");
        if constexpr (std::is_same_v<T, u32>)
        {
            _v = DirectX::XMVectorSetInt(x, y, z, w);
        }
        else
        {
            _v = DirectX::XMVectorSet(x, y, z, w);
        }
    }

    template<RegisterType T, u32 Dim> requires ValidVectorDim<Dim>
    inline TVectorRegister<T, Dim>::TVectorRegister(const TVectorRegister<T, Dim>& other) noexcept
        : _v(other._v)
    {
    }

    template<RegisterType T, u32 Dim> requires ValidVectorDim<Dim>
    inline TVectorRegister<T, Dim> TVectorRegister<T, Dim>::operator-() const
    {
        TVectorRegister<T, Dim> v;
        v._v = DirectX::XMVectorNegate(_v);
        return v;
    }

    template<RegisterType T, u32 Dim> requires ValidVectorDim<Dim>
    inline TVectorRegister<T, Dim>& TVectorRegister<T, Dim>::operator=(const TVectorRegister<T, Dim>& other)
    {
        if (this == &other)
        {
            return *this;
        }

        _v = other._v;
        return *this;
    }

    template<RegisterType T, u32 Dim> requires ValidVectorDim<Dim>
    inline TVectorRegister<T, Dim> TVectorRegister<T, Dim>::operator+(const TVectorRegister<T, Dim>& other) const
    {
        TVectorRegister<T, Dim> v;
        v._v = DirectX::XMVectorAdd(_v, other._v);
        return v;
    }

    template<RegisterType T, u32 Dim> requires ValidVectorDim<Dim>
    inline TVectorRegister<T, Dim>& TVectorRegister<T, Dim>::operator+=(const TVectorRegister<T, Dim>& other)
    {
        _v = DirectX::XMVectorAdd(_v, other._v);
        return *this;
    }

    template<RegisterType T, u32 Dim> requires ValidVectorDim<Dim>
    inline TVectorRegister<T, Dim> TVectorRegister<T, Dim>::operator+(T scalar) const
    {
        if constexpr (Dim == 2)
        {
            TVectorRegister<T, Dim> s(scalar, scalar);
            TVectorRegister<T, Dim> v;
            v._v = DirectX::XMVectorAdd(_v, s._v);
            return v;
        }
        else if constexpr (Dim == 3)
        {
            TVectorRegister<T, Dim> s(scalar, scalar, scalar);
            TVectorRegister<T, Dim> v;
            v._v = DirectX::XMVectorAdd(_v, s._v);
            return v;
        }
        else if constexpr (Dim == 4)
        {
            TVectorRegister<T, Dim> s(scalar, scalar, scalar, scalar);
            TVectorRegister<T, Dim> v;
            v._v = DirectX::XMVectorAdd(_v, s._v);
            return v;
        }
        return 0;
    }

    template<RegisterType T, u32 Dim> requires ValidVectorDim<Dim>
    inline TVectorRegister<T, Dim>& TVectorRegister<T, Dim>::operator+=(T scalar)
    {
        if constexpr (Dim == 2)
        {
            TVectorRegister<T, Dim> s(scalar, scalar);
            _v = DirectX::XMVectorAdd(_v, s._v);
        }
        else if constexpr (Dim == 3)
        {
            TVectorRegister<T, Dim> s(scalar, scalar, scalar);
            _v = DirectX::XMVectorAdd(_v, s._v);
        }
        else if constexpr (Dim == 4)
        {
            TVectorRegister<T, Dim> s(scalar, scalar, scalar, scalar);
            _v = DirectX::XMVectorAdd(_v, s._v);
        }
        return *this;
    }

    template<RegisterType T, u32 Dim> requires ValidVectorDim<Dim>
    inline TVectorRegister<T, Dim> TVectorRegister<T, Dim>::operator-(const TVectorRegister<T, Dim>& other) const
    {
        TVectorRegister<T, Dim> v;
        v._v = DirectX::XMVectorSubtract(_v, other._v);
        return v;
    }

    template<RegisterType T, u32 Dim> requires ValidVectorDim<Dim>
    inline TVectorRegister<T, Dim>& TVectorRegister<T, Dim>::operator-=(const TVectorRegister<T, Dim>& other)
    {
        _v = DirectX::XMVectorSubtract(_v, other._v);
        return *this;
    }

    template<RegisterType T, u32 Dim> requires ValidVectorDim<Dim>
    inline TVectorRegister<T, Dim> TVectorRegister<T, Dim>::operator-(T scalar) const
    {
        if constexpr (Dim == 2)
        {
            TVectorRegister<T, Dim> s(scalar, scalar);
            TVectorRegister<T, Dim> v;
            v._v = DirectX::XMVectorSubtract(_v, s._v);
            return v;
        }
        else if constexpr (Dim == 3)
        {
            TVectorRegister<T, Dim> s(scalar, scalar, scalar);
            TVectorRegister<T, Dim> v;
            v._v = DirectX::XMVectorSubtract(_v, s._v);
            return v;
        }
        else if constexpr (Dim == 4)
        {
            TVectorRegister<T, Dim> s(scalar, scalar, scalar, scalar);
            TVectorRegister<T, Dim> v;
            v._v = DirectX::XMVectorSubtract(_v, s._v);
            return v;
        }
        return 0;
    }

    template<RegisterType T, u32 Dim> requires ValidVectorDim<Dim>
    inline TVectorRegister<T, Dim>& TVectorRegister<T, Dim>::operator-=(T scalar)
    {
        if constexpr (Dim == 2)
        {
            TVectorRegister<T, Dim> s(scalar, scalar);
            _v = DirectX::XMVectorSubtract(_v, s._v);
        }
        else if constexpr (Dim == 3)
        {
            TVectorRegister<T, Dim> s(scalar, scalar, scalar);
            _v = DirectX::XMVectorSubtract(_v, s._v);
        }
        else if constexpr (Dim == 4)
        {
            TVectorRegister<T, Dim> s(scalar, scalar, scalar, scalar);
            _v = DirectX::XMVectorSubtract(_v, s._v);
        }
        return *this;
    }

    template<RegisterType T, u32 Dim> requires ValidVectorDim<Dim>
    inline TVectorRegister<T, Dim> TVectorRegister<T, Dim>::operator*(const TVectorRegister<T, Dim>& other) const
    {
        TVectorRegister<T, Dim> v;
        v._v = DirectX::XMVectorMultiply(_v, other._v);
        return v;
    }

    template<RegisterType T, u32 Dim> requires ValidVectorDim<Dim>
    inline TVectorRegister<T, Dim>& TVectorRegister<T, Dim>::operator*=(const TVectorRegister<T, Dim>& other)
    {
        _v = DirectX::XMVectorMultiply(_v, other._v);
        return *this;
    }

    template<RegisterType T, u32 Dim> requires ValidVectorDim<Dim>
    inline TVectorRegister<T, Dim> TVectorRegister<T, Dim>::operator*(T scalar) const
    {
        TVectorRegister<T, Dim> v;
        v._v = DirectX::XMVectorScale(_v, scalar);
        return v;
    }

    template<RegisterType T, u32 Dim> requires ValidVectorDim<Dim>
    inline TVectorRegister<T, Dim>& TVectorRegister<T, Dim>::operator*=(T scalar)
    {
        _v = DirectX::XMVectorScale(_v, scalar);
        return *this;
    }

    template<RegisterType T, u32 Dim> requires ValidVectorDim<Dim>
    inline TVectorRegister<T, Dim> TVectorRegister<T, Dim>::operator/(const TVectorRegister<T, Dim>& other) const
    {
        TVectorRegister<T, Dim> v;
        v._v = DirectX::XMVectorDivide(_v, other._v);
        return v;
    }

    template<RegisterType T, u32 Dim> requires ValidVectorDim<Dim>
    inline TVectorRegister<T, Dim>& TVectorRegister<T, Dim>::operator/=(const TVectorRegister<T, Dim>& other)
    {
        _v = DirectX::XMVectorDivide(_v, other._v);
        return *this;
    }

    template<RegisterType T, u32 Dim> requires ValidVectorDim<Dim>
    inline TVectorRegister<T, Dim> TVectorRegister<T, Dim>::operator/(T scalar) const
    {
        if constexpr (Dim == 2)
        {
            TVectorRegister<T, Dim> s(scalar, scalar);
            TVectorRegister<T, Dim> v;
            v._v = DirectX::XMVectorDivide(_v, s._v);
            return v;
        }
        else if constexpr (Dim == 3)
        {
            TVectorRegister<T, Dim> s(scalar, scalar, scalar);
            TVectorRegister<T, Dim> v;
            v._v = DirectX::XMVectorDivide(_v, s._v);
            return v;
        }
        else if constexpr (Dim == 4)
        {
            TVectorRegister<T, Dim> s(scalar, scalar, scalar, scalar);
            TVectorRegister<T, Dim> v;
            v._v = DirectX::XMVectorDivide(_v, s._v);
            return v;
        }
        return *this;
    }

    template<RegisterType T, u32 Dim> requires ValidVectorDim<Dim>
    inline TVectorRegister<T, Dim>& TVectorRegister<T, Dim>::operator/=(T scalar)
    {
        if constexpr (Dim == 2)
        {
            TVectorRegister<T, Dim> s(scalar, scalar);
            _v = DirectX::XMVectorDivide(_v, s._v);
        }
        else if constexpr (Dim == 3)
        {
            TVectorRegister<T, Dim> s(scalar, scalar, scalar);
            _v = DirectX::XMVectorDivide(_v, s._v);
        }
        else if constexpr (Dim == 4)
        {
            TVectorRegister<T, Dim> s(scalar, scalar, scalar, scalar);
            _v = DirectX::XMVectorDivide(_v, s._v);
        }
        return *this;
    }

    template<RegisterType T, u32 Dim> requires ValidVectorDim<Dim>
    inline bool TVectorRegister<T, Dim>::operator<=(const TVectorRegister<T, Dim>& other) const
    {
        return DirectX::XMVectorLessOrEqual(_v, other._v);
    }

    template<RegisterType T, u32 Dim> requires ValidVectorDim<Dim>
    inline bool TVectorRegister<T, Dim>::operator>=(const TVectorRegister<T, Dim>& other) const
    {
        return DirectX::XMVectorGreaterOrEqual(_v, other._v);
    }

    template<RegisterType T, u32 Dim> requires ValidVectorDim<Dim>
    inline bool TVectorRegister<T, Dim>::operator<(const TVectorRegister<T, Dim>& other) const
    {
        return DirectX::XMVectorLess(_v, other._v);
    }

    template<RegisterType T, u32 Dim> requires ValidVectorDim<Dim>
    inline bool TVectorRegister<T, Dim>::operator>(const TVectorRegister<T, Dim>& other) const
    {
        return DirectX::XMVectorGreater(_v, other._v);
    }

    template<RegisterType T, u32 Dim> requires ValidVectorDim<Dim>
    inline bool TVectorRegister<T, Dim>::operator==(const TVectorRegister<T, Dim>& other) const
    {
        if constexpr (std::is_same_v<T, u32>)
        {
            if constexpr (Dim == 2)
            {
                return DirectX::XMVector2EqualInt(_v, other._v);
            }
            else if constexpr (Dim == 3)
            {
                return DirectX::XMVector2EqualInt(_v, other._v);
            }
            else if constexpr (Dim == 4)
            {
                return DirectX::XMVector2EqualInt(_v, other._v);
            }
        }
        else
        {
            if constexpr (Dim == 2)
            {
                return DirectX::XMVector2Equal(_v, other._v);
            }
            else if constexpr (Dim == 3)
            {
                return DirectX::XMVector3Equal(_v, other._v);
            }
            else if constexpr (Dim == 4)
            {
                return DirectX::XMVector4Equal(_v, other._v);
            }
        }

        return false;
    }

    template<RegisterType T, u32 Dim> requires ValidVectorDim<Dim>
    inline bool TVectorRegister<T, Dim>::operator!=(const TVectorRegister<T, Dim>& other) const
    {
        return DirectX::XMVectorNotEqual(_v, other._v);
    }

    template<RegisterType T, u32 Dim> requires ValidVectorDim<Dim>
    inline T TVectorRegister<T, Dim>::operator[](u32 index) const
    {
        if constexpr (std::is_same_v<T, u32>)
        {
            return DirectX::XMVectorGetIntByIndex(_v, index);
        }
        else
        {
            return DirectX::XMVectorGetByIndex(_v, index);
        }
    }

    template<RegisterType T, u32 Dim> requires ValidVectorDim<Dim>
    inline TVectorRegister<T, Dim>& TVectorRegister<T, Dim>::set(T x, T y)
    {
        static_assert(Dim == 2, "Unsupported dimension");
        if constexpr (std::is_same_v<T, u32>)
        {
            _v = DirectX::XMVectorSetInt(x, y, 0, 0);
        }
        else
        {
            _v = DirectX::XMVectorSet(x, y, k_NaN, k_NaN);
        }

        return *this;
    }

    template<RegisterType T, u32 Dim> requires ValidVectorDim<Dim>
    inline TVectorRegister<T, Dim>& TVectorRegister<T, Dim>::set(T x, T y, T z)
    {
        static_assert(Dim == 3, "Unsupported dimension");
        if constexpr (std::is_same_v<T, u32>)
        {
            _v = DirectX::XMVectorSetInt(x, y, z, 0);
        }
        else
        {
            _v = DirectX::XMVectorSet(x, y, z, k_NaN);
        }

        return *this;
    }

    template<RegisterType T, u32 Dim> requires ValidVectorDim<Dim>
    inline TVectorRegister<T, Dim>& TVectorRegister<T, Dim>::set(T x, T y, T z, T w)
    {
        static_assert(Dim == 4, "Unsupported dimension");
        if constexpr (std::is_same_v<T, u32>)
        {
            _v = DirectX::XMVectorSetInt(x, y, z, w);
        }
        else
        {
            _v = DirectX::XMVectorSet(x, y, z, w);
        }

        return *this;
    }

    template<RegisterType T, u32 Dim> requires ValidVectorDim<Dim>
    inline TVectorRegister<T, Dim>& TVectorRegister<T, Dim>::set(const TVectorRegister<T, Dim>& other)
    {
        _v = other._v;
        return *this;
    }

    template<RegisterType T, u32 Dim> requires ValidVectorDim<Dim>
    inline T TVectorRegister<T, Dim>::getX() const
    {
        static_assert(Dim >= 2, "Unsupported dimension");
        return this->operator[](0);
    }

    template<RegisterType T, u32 Dim> requires ValidVectorDim<Dim>
    inline T TVectorRegister<T, Dim>::getY() const
    {
        static_assert(Dim >= 2, "Unsupported dimension");
        return this->operator[](1);
    }

    template<RegisterType T, u32 Dim> requires ValidVectorDim<Dim>
    inline T TVectorRegister<T, Dim>::getZ() const
    {
        static_assert(Dim >= 3, "Unsupported dimension");
        return this->operator[](2);
    }

    template<RegisterType T, u32 Dim> requires ValidVectorDim<Dim>
    inline T TVectorRegister<T, Dim>::getW() const
    {
        static_assert(Dim >= 4, "Unsupported dimension");
        return this->operator[](3);
    }

    template<RegisterType T, u32 Dim> requires ValidVectorDim<Dim>
    inline void TVectorRegister<T, Dim>::setX(T x)
    {
        static_assert(Dim >= 2, "Unsupported dimension");
        if constexpr (std::is_same_v<T, u32>)
        {
            _v = DirectX::XMVectorSetIntX(_v, x);
        }
        else
        {
            _v = DirectX::XMVectorSetX(_v, x);
        }
    }

    template<RegisterType T, u32 Dim> requires ValidVectorDim<Dim>
    inline void TVectorRegister<T, Dim>::setY(T y)
    {
        static_assert(Dim >= 2, "Unsupported dimension");
        if constexpr (std::is_same_v<T, u32>)
        {
            _v = DirectX::XMVectorSetIntY(_v, y);
        }
        else
        {
            _v = DirectX::XMVectorSetY(_v, y);
        }
    }

    template<RegisterType T, u32 Dim> requires ValidVectorDim<Dim>
    inline void TVectorRegister<T, Dim>::setZ(T z)
    {
        static_assert(Dim >= 3, "Unsupported dimension");
        if constexpr (std::is_same_v<T, u32>)
        {
            _v = DirectX::XMVectorSetIntZ(_v, z);
        }
        else
        {
            _v = DirectX::XMVectorSetZ(_v, z);
        }
    }

    template<RegisterType T, u32 Dim> requires ValidVectorDim<Dim>
    inline void TVectorRegister<T, Dim>::setW(T w)
    {
        static_assert(Dim >= 4, "Unsupported dimension");
        if constexpr (std::is_same_v<T, u32>)
        {
            _v = DirectX::XMVectorSetIntW(_v, w);
        }
        else
        {
            _v = DirectX::XMVectorSetW(_v, w);
        }
    }

    template<RegisterType T, u32 Dim> requires ValidVectorDim<Dim>
    inline T TVectorRegister<T, Dim>::length() const
    {
        if constexpr (Dim == 2)
        {
            return DirectX::XMVectorGetX(DirectX::XMVector2Length(_v));
        }
        else if constexpr (Dim == 3)
        {
            return DirectX::XMVectorGetX(DirectX::XMVector3Length(_v));
        }
        else if constexpr (Dim == 4)
        {
            return DirectX::XMVectorGetX(DirectX::XMVector4Length(_v));
        }

        return 0;
    }

    template<RegisterType T, u32 Dim> requires ValidVectorDim<Dim>
    inline T TVectorRegister<T, Dim>::lengthSQ() const
    {
        if constexpr (Dim == 2)
        {
            return DirectX::XMVectorGetX(DirectX::XMVector4LengthSq(_v));
        }
        else if constexpr (Dim == 3)
        {
            return DirectX::XMVectorGetX(DirectX::XMVector4LengthSq(_v));
        }
        else if constexpr (Dim == 4)
        {
            return DirectX::XMVectorGetX(DirectX::XMVector4LengthSq(_v));
        }

        return 0;
    }

    template<RegisterType T, u32 Dim> requires ValidVectorDim<Dim>
    inline T TVectorRegister<T, Dim>::distanceFrom(const TVectorRegister<T, Dim>& other) const
    {
        TVectorRegister<T, Dim> v;
        v._v = DirectX::XMVectorSubtract(_v, other._v);
        const T distance = v.length();
        return distance;
    }

    template<RegisterType T, u32 Dim> requires ValidVectorDim<Dim>
    inline T TVectorRegister<T, Dim>::distanceFromSQ(const TVectorRegister<T, Dim>& other) const
    {
        TVectorRegister<T, Dim> v;
        v._v = DirectX::XMVectorSubtract(_v, other._v);
        const T distanceSQ = v.lengthSQ();
        return distanceSQ;
    }

    template<RegisterType T, u32 Dim> requires ValidVectorDim<Dim>
    inline bool TVectorRegister<T, Dim>::isEquals(const TVectorRegister& other, f32 tolerance) const
    {
        if constexpr (std::is_same_v<T, u32>)
        {
            return DirectX::XMVectorEqualInt(_v, other._v);
        }
        else
        {
            TVectorRegister<f32> toleranceVec(tolerance, tolerance, tolerance, tolerance);
            return DirectX::XMVectorNearEqual(_v, other._v, toleranceVec);
        }
    }

    template<RegisterType T, u32 Dim> requires ValidVectorDim<Dim>
    inline TVectorRegister<T, Dim>& TVectorRegister<T, Dim>::normalize()
    {
        if constexpr (Dim == 2)
        {
            _v = DirectX::XMVector2Normalize(_v);
        }
        else if constexpr (Dim == 3)
        {
            _v = DirectX::XMVector3Normalize(_v);
        }
        else if constexpr (Dim == 4)
        {
            _v = DirectX::XMVector4Normalize(_v);
        }
        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<RegisterType T, u32 Dim>
    inline T SVector::distance(const TVectorRegister<T, Dim>& v1, const TVectorRegister<T, Dim>& v2)
    {
        TVectorRegister<T, Dim> v = v1 - v2;
        return v.length();
    }

    template<RegisterType T, u32 Dim>
    inline T SVector::dot(const TVectorRegister<T, Dim>& v1, const TVectorRegister<T, Dim>& v2)
    {
        if constexpr (Dim == 2)
        {
            return DirectX::XMVectorGetX(DirectX::XMVector2Dot(v1._v, v2._v));
        }
        else if constexpr (Dim == 3)
        {
            return DirectX::XMVectorGetX(DirectX::XMVector3Dot(v1._v, v2._v));
        }
        else if constexpr (Dim == 4)
        {
            return DirectX::XMVectorGetX(DirectX::XMVector4Dot(v1._v, v2._v));
        }

        return 0;
    }

    template<RegisterType T, u32 Dim>
    inline TVectorRegister<T, Dim> SVector::cross(const TVectorRegister<T, Dim>& v1, const TVectorRegister<T, Dim>& v2)
    {
        if constexpr (Dim == 2)
        {
            TVectorRegister<T, Dim> v;
            v._v = DirectX::XMVector2Cross(v1._v, v2._v);
            return v;
        }
        else if constexpr (Dim == 3)
        {
            TVectorRegister<T, Dim> v;
            v._v = DirectX::XMVector3Cross(v1._v, v2._v);
            return v;
        }
        else if constexpr (Dim == 4)
        {
            TVectorRegister<T, Dim> v;
            v._v = DirectX::XMVector4Cross(v1._v, v2._v);
            return v;
        }

        return 0;
    }

    template<RegisterType T, u32 Dim>
    inline TVectorRegister<T, Dim> SVector::lerp(const TVectorRegister<T, Dim>& v1, const TVectorRegister<T, Dim>& v2, const T d)
    {
        TVectorRegister<T, Dim> v;
        v._v = DirectX::XMVectorLerp(v1._v, v2._v, d);
        return v;
    }

    template<typename S, RegisterType T, u32 Dim>
    inline TVectorRegister<T, Dim> operator*(S scalar, const TVectorRegister<T, Dim>& vector)
    {
        return vector * scalar;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace math
} //namespace v3d
