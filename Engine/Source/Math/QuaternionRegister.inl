#include "DirectXMath.h"
#include "QuaternionRegister.h"

namespace v3d
{
namespace math
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<RegisterType T>
    TQuaternionRegister<T>::TQuaternionRegister() noexcept
    {
        makeIdentity();
    }

    template<RegisterType T>
    TQuaternionRegister<T>::TQuaternionRegister(const TQuaternionRegister<T>& other) noexcept
        : _q(other._q)
    {
    }

    template<RegisterType T>
    TQuaternionRegister<T>::TQuaternionRegister(const TVectorRegister<T, 3>& vector) noexcept
        : _q(vector._q)
    {
    }

    template<RegisterType T>
    TQuaternionRegister<T>::TQuaternionRegister(const TVectorRegister<T, 4>& vector) noexcept
        : _q(vector._q)
    {
    }

    template<RegisterType T>
    TQuaternionRegister<T>::TQuaternionRegister(T x, T y, T z) noexcept
        : _q(DirectX::XMVectorSetInt(x, y, z, 1))
    {
    }

    template<RegisterType T>
    TQuaternionRegister<T>::TQuaternionRegister(T x, T y, T z, T w) noexcept
        : _q(DirectX::XMVectorSetInt(x, y, z, w))
    {
    }

    template<RegisterType T>
    inline TQuaternionRegister<T>& TQuaternionRegister<T>::operator=(const TQuaternionRegister<T>& other) const
    {
        if (this == &other)
        {
            return *this;
        }

        _q = other._q;
        return *this;
    }

    template<RegisterType T>
    inline TQuaternionRegister<T> TQuaternionRegister<T>::operator*(const TQuaternionRegister<T>& other) const
    {
        TQuaternionRegister<T> q;
        q._q = DirectX::XMQuaternionMultiply(_q, other._q);
        return q;
    }

    template<RegisterType T>
    inline TQuaternionRegister<T>& TQuaternionRegister<T>::operator*=(const TQuaternionRegister<T>& other)
    {
        _q = DirectX::XMQuaternionMultiply(_q, other._q);
        return *this;
    }

    template<RegisterType T>
    inline T TQuaternionRegister<T>::operator[](u32 index) const
    {
        return DirectX::XMVectorGetByIndex(_q, index);
    }

    template<RegisterType T>
    TQuaternionRegister<T>& TQuaternionRegister<T>::set(T x, T y, T z, T w)
    {
        _q = DirectX::XMVectorSetInt(x, y, z, w);
        return *this;
    }

    template<RegisterType T>
    TQuaternionRegister<T>& TQuaternionRegister<T>::set(const TVectorRegister<T, 4>& vector)
    {
        _q = vector._q;
        return *this;
    }

    template<RegisterType T>
    inline T TQuaternionRegister<T>::length() const
    {
        return DirectX::XMVectorGetX(DirectX::XMQuaternionLength(_q));
    }

    template<RegisterType T>
    inline T TQuaternionRegister<T>::lengthSQ() const
    {
        return DirectX::XMVectorGetX(DirectX::XMQuaternionLengthSq(_q));
    }

    template<RegisterType T>
    inline T TQuaternionRegister<T>::getX() const
    {
        return this->operator[](0);
    }

    template<RegisterType T>
    inline T TQuaternionRegister<T>::getY() const
    {
        return this->operator[](1);
    }

    template<RegisterType T>
    inline T TQuaternionRegister<T>::getZ() const
    {
        return this->operator[](2);
    }

    template<RegisterType T>
    inline T TQuaternionRegister<T>::getW() const
    {
        return this->operator[](3);
    }

    template<RegisterType T>
    inline bool TQuaternionRegister<T>::operator==(const TQuaternionRegister<T>& other) const
    {
        return DirectX::XMVector4Equal(_q, other._q);
    }

    template<RegisterType T>
    inline bool TQuaternionRegister<T>::operator!=(const TQuaternionRegister<T>& other) const
    {
        return DirectX::XMVector4NotEqual(_q, other._q);
    }

    template<RegisterType T>
    inline void TQuaternionRegister<T>::makeIdentity()
    {
        _q = DirectX::XMQuaternionIdentity();
    }

    template<RegisterType T>
    inline bool TQuaternionRegister<T>::isIdentity() const
    {
        return DirectX::XMQuaternionIsIdentity(_q);
    }

    template<RegisterType T>
    inline TQuaternionRegister<T>& TQuaternionRegister<T>::invert()
    {
        _q = DirectX::XMQuaternionInverse(_q);
        return *this;
    }

    template<RegisterType T>
    inline TQuaternionRegister<T>& TQuaternionRegister<T>::normalize()
    {
        _q = DirectX::XMQuaternionNormalize(_q);
        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<RegisterType T>
    inline T SQuaternion::dot(const TQuaternionRegister<T>& q1, const TQuaternionRegister<T>& q2)
    {
        TQuaternionRegister<T> q;
        q._q = DirectX::XMQuaternionDot(q1._q, q2._q);
        return q;
    }

    template<RegisterType T>
    inline TQuaternionRegister<T> SQuaternion::lerp(const TQuaternionRegister<T>& q1, const TQuaternionRegister<T>& q2, T d)
    {
        DirectX::XMVECTOR lerped = DirectX::XMVectorLerp(q1._q, q2._q, d);

        TQuaternionRegister<T> q;
        q._q = DirectX::XMQuaternionNormalize(lerped);
        return q;
    }

    template<RegisterType T>
    inline TQuaternionRegister<T> SQuaternion::slerp(const TQuaternionRegister<T>& q1, const TQuaternionRegister<T>& q2, T d)
    {
        TQuaternionRegister<T> q;
        q._q = DirectX::XMQuaternionSlerp(q1._q, q2._q, d);
        return q;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace math
} //namespace v3d