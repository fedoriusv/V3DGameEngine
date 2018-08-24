#pragma once

#include "Math.h"
#include "Vector2D.h"

namespace v3d
{
namespace core
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template <class T>
    class TMatrix2D
    {
    public:

        TMatrix2D();
        TMatrix2D(const TMatrix2D<T>& other);
        TMatrix2D(const T* matrix);
        TMatrix2D(T m0, T m1, T m2, T m3);

        T&              operator () (u32 row, u32 col);
        const T&        operator () (u32 row, u32 col)              const;
        T&              operator [] (u32 index);
        const T&        operator [] (u32 index)                     const;
        TMatrix2D<T>&   operator =  (const T& scalar);
        bool            operator == (const TMatrix2D<T> &other)     const;
        bool            operator != (const TMatrix2D<T> &other)     const;
        TMatrix2D<T>    operator +  (const TMatrix2D<T>& other)     const;
        TMatrix2D<T>&   operator += (const TMatrix2D<T>& other);
        TMatrix2D<T>    operator -  (const TMatrix2D<T>& other)     const;
        TMatrix2D<T>&   operator -= (const TMatrix2D<T>& other);
        TMatrix2D<T>    operator *  (const TMatrix2D<T>& other)     const;
        TMatrix2D<T>&   operator *= (const TMatrix2D<T>& other);
        TMatrix2D<T>    operator *  (const T& scalar)               const;
        TMatrix2D<T>&   operator *= (const T& scalar);

        TMatrix2D<T>&   set(const T* data);

        T*              getPtr();
        const T*        getPtr()                                    const;

        void            makeIdentity();
        bool            isIdentity()                                const;

        void            setRotation(T angle);
        T               getRotation()                               const;

        void            preScale(const TVector2D<T>& scale);
        void            postScale(const TVector2D<T>& scale);
        void            setScale(const TVector2D<T>& scale);

        TVector2D<T>    getScale()                                  const;

        bool            makeInverse();
        bool            getInverse(TMatrix2D<T>& out)               const;

        void            makeTransposed();
        TMatrix2D<T>    getTransposed()                             const;

    private:

        T m_matrix[4];
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    using Matrix2D = TMatrix2D<f32>;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace core
} //namespace v3d

#include "Matrix2D.inl"
