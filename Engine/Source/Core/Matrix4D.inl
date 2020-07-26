#include "Matrix4D.h"
namespace v3d
{
namespace core
{

    template <class T>
    TMatrix4D<T>::TMatrix4D()
    {
        makeIdentity();
    }

    template <class T>
    TMatrix4D<T>::TMatrix4D(const TMatrix4D<T>& other)
    {
        *this = other;
    }

    template <class T>
    TMatrix4D<T>::TMatrix4D(const T* matrix)
    {
        memcpy(m_matrix, matrix, 16 * sizeof(T));
    }

    template <class T>
    TMatrix4D<T>::TMatrix4D(const TVector4D<T>& col0,
        const TVector4D<T>& col1,
        const TVector4D<T>& col2,
        const TVector4D<T>& col3)
    {
        m_matrix[0] = col0.x;
        m_matrix[1] = col0.y;
        m_matrix[2] = col0.z;
        m_matrix[3] = col0.w;
        m_matrix[4] = col1.x;
        m_matrix[5] = col1.y;
        m_matrix[6] = col1.z;
        m_matrix[7] = col1.w;
        m_matrix[8] = col2.x;
        m_matrix[9] = col2.y;
        m_matrix[10] = col2.z;
        m_matrix[11] = col2.w;
        m_matrix[12] = col3.x;
        m_matrix[13] = col3.y;
        m_matrix[14] = col3.z;
        m_matrix[15] = col3.w;
    }

    template <class T>
    TMatrix4D<T>::TMatrix4D(const T& m0, const T& m1, const T& m2, const T& m3,
        const T& m4, const T& m5, const T& m6, const T& m7,
        const T& m8, const T& m9, const T& m10, const T& m11,
        const T& m12, const T& m13, const T& m14, const T& m15)
    {
        m_matrix[0] = m0;
        m_matrix[1] = m1;
        m_matrix[2] = m2;
        m_matrix[3] = m3;
        m_matrix[4] = m4;
        m_matrix[5] = m5;
        m_matrix[6] = m6;
        m_matrix[7] = m7;
        m_matrix[8] = m8;
        m_matrix[9] = m9;
        m_matrix[10] = m10;
        m_matrix[11] = m11;
        m_matrix[12] = m12;
        m_matrix[13] = m13;
        m_matrix[14] = m14;
        m_matrix[15] = m15;
    }

    template <class T>
    T& TMatrix4D<T>::operator()(u32 row, u32 col)
    {
        return m_matrix[row * 4 + col];
    }

    template <class T>
    const T& TMatrix4D<T>::operator()(u32 row, u32 col) const
    {
        return m_matrix[row * 4 + col];
    }

    template <class T>
    T& TMatrix4D<T>::operator[](u32 index)
    {
        return m_matrix[index];
    }

    template <class T>
    const T& TMatrix4D<T>::operator[](u32 index) const
    {
        return m_matrix[index];
    }


    template <class T>
    TMatrix4D<T> TMatrix4D<T>::operator+(const TMatrix4D<T>& other) const
    {
        TMatrix4D<T> temp;

        temp[0] = m_matrix[0] + other[0];
        temp[1] = m_matrix[1] + other[1];
        temp[2] = m_matrix[2] + other[2];
        temp[3] = m_matrix[3] + other[3];
        temp[4] = m_matrix[4] + other[4];
        temp[5] = m_matrix[5] + other[5];
        temp[6] = m_matrix[6] + other[6];
        temp[7] = m_matrix[7] + other[7];
        temp[8] = m_matrix[8] + other[8];
        temp[9] = m_matrix[9] + other[9];
        temp[10] = m_matrix[10] + other[10];
        temp[11] = m_matrix[11] + other[11];
        temp[12] = m_matrix[12] + other[12];
        temp[13] = m_matrix[13] + other[13];
        temp[14] = m_matrix[14] + other[14];
        temp[15] = m_matrix[15] + other[15];

        return temp;
    }

    template <class T>
    TMatrix4D<T>& TMatrix4D<T>::operator+=(const TMatrix4D<T>& other)
    {
        m_matrix[0] += other[0];
        m_matrix[1] += other[1];
        m_matrix[2] += other[2];
        m_matrix[3] += other[3];
        m_matrix[4] += other[4];
        m_matrix[5] += other[5];
        m_matrix[6] += other[6];
        m_matrix[7] += other[7];
        m_matrix[8] += other[8];
        m_matrix[9] += other[9];
        m_matrix[10] += other[10];
        m_matrix[11] += other[11];
        m_matrix[12] += other[12];
        m_matrix[13] += other[13];
        m_matrix[14] += other[14];
        m_matrix[15] += other[15];

        return *this;
    }

    template <class T>
    TMatrix4D<T> TMatrix4D<T>::operator-(const TMatrix4D<T>& other) const
    {
        TMatrix4D<T> temp;

        temp[0] = m_matrix[0] - other[0];
        temp[1] = m_matrix[1] - other[1];
        temp[2] = m_matrix[2] - other[2];
        temp[3] = m_matrix[3] - other[3];
        temp[4] = m_matrix[4] - other[4];
        temp[5] = m_matrix[5] - other[5];
        temp[6] = m_matrix[6] - other[6];
        temp[7] = m_matrix[7] - other[7];
        temp[8] = m_matrix[8] - other[8];
        temp[9] = m_matrix[9] - other[9];
        temp[10] = m_matrix[10] - other[10];
        temp[11] = m_matrix[11] - other[11];
        temp[12] = m_matrix[12] - other[12];
        temp[13] = m_matrix[13] - other[13];
        temp[14] = m_matrix[14] - other[14];
        temp[15] = m_matrix[15] - other[15];

        return temp;
    }

    template <class T>
    TMatrix4D<T>& TMatrix4D<T>::operator-=(const TMatrix4D<T>& other)
    {

        m_matrix[0] -= other[0];
        m_matrix[1] -= other[1];
        m_matrix[2] -= other[2];
        m_matrix[3] -= other[3];
        m_matrix[4] -= other[4];
        m_matrix[5] -= other[5];
        m_matrix[6] -= other[6];
        m_matrix[7] -= other[7];
        m_matrix[8] -= other[8];
        m_matrix[9] -= other[9];
        m_matrix[10] -= other[10];
        m_matrix[11] -= other[11];
        m_matrix[12] -= other[12];
        m_matrix[13] -= other[13];
        m_matrix[14] -= other[14];
        m_matrix[15] -= other[15];

        return *this;
    }

    template <class T>
    TMatrix4D<T> TMatrix4D<T>::operator*(const T& scalar) const
    {
        TMatrix4D<T> temp;

        temp[0] = m_matrix[0] * scalar;
        temp[1] = m_matrix[1] * scalar;
        temp[2] = m_matrix[2] * scalar;
        temp[3] = m_matrix[3] * scalar;
        temp[4] = m_matrix[4] * scalar;
        temp[5] = m_matrix[5] * scalar;
        temp[6] = m_matrix[6] * scalar;
        temp[7] = m_matrix[7] * scalar;
        temp[8] = m_matrix[8] * scalar;
        temp[9] = m_matrix[9] * scalar;
        temp[10] = m_matrix[10] * scalar;
        temp[11] = m_matrix[11] * scalar;
        temp[12] = m_matrix[12] * scalar;
        temp[13] = m_matrix[13] * scalar;
        temp[14] = m_matrix[14] * scalar;
        temp[15] = m_matrix[15] * scalar;

        return temp;
    }

    template <class T>
    TMatrix4D<T>& TMatrix4D<T>::operator*=(const T& scalar)
    {
        m_matrix[0] *= scalar;
        m_matrix[1] *= scalar;
        m_matrix[2] *= scalar;
        m_matrix[3] *= scalar;
        m_matrix[4] *= scalar;
        m_matrix[5] *= scalar;
        m_matrix[6] *= scalar;
        m_matrix[7] *= scalar;
        m_matrix[8] *= scalar;
        m_matrix[9] *= scalar;
        m_matrix[10] *= scalar;
        m_matrix[11] *= scalar;
        m_matrix[12] *= scalar;
        m_matrix[13] *= scalar;
        m_matrix[14] *= scalar;
        m_matrix[15] *= scalar;

        return *this;
    }

    template <class T>
    TMatrix4D<T>& TMatrix4D<T>::operator*=(const TMatrix4D<T>& other)
    {
        TMatrix4D<T> temp(*this);

        const T * m1 = temp.m_matrix;
        const T * m2 = other.m_matrix;

        m_matrix[0] = m1[0] * m2[0] + m1[4] * m2[1] + m1[8] * m2[2] + m1[12] * m2[3];
        m_matrix[1] = m1[1] * m2[0] + m1[5] * m2[1] + m1[9] * m2[2] + m1[13] * m2[3];
        m_matrix[2] = m1[2] * m2[0] + m1[6] * m2[1] + m1[10] * m2[2] + m1[14] * m2[3];
        m_matrix[3] = m1[3] * m2[0] + m1[7] * m2[1] + m1[11] * m2[2] + m1[15] * m2[3];
        m_matrix[4] = m1[0] * m2[4] + m1[4] * m2[5] + m1[8] * m2[6] + m1[12] * m2[7];
        m_matrix[5] = m1[1] * m2[4] + m1[5] * m2[5] + m1[9] * m2[6] + m1[13] * m2[7];
        m_matrix[6] = m1[2] * m2[4] + m1[6] * m2[5] + m1[10] * m2[6] + m1[14] * m2[7];
        m_matrix[7] = m1[3] * m2[4] + m1[7] * m2[5] + m1[11] * m2[6] + m1[15] * m2[7];
        m_matrix[8] = m1[0] * m2[8] + m1[4] * m2[9] + m1[8] * m2[10] + m1[12] * m2[11];
        m_matrix[9] = m1[1] * m2[8] + m1[5] * m2[9] + m1[9] * m2[10] + m1[13] * m2[11];
        m_matrix[10] = m1[2] * m2[8] + m1[6] * m2[9] + m1[10] * m2[10] + m1[14] * m2[11];
        m_matrix[11] = m1[3] * m2[8] + m1[7] * m2[9] + m1[11] * m2[10] + m1[15] * m2[11];
        m_matrix[12] = m1[0] * m2[12] + m1[4] * m2[13] + m1[8] * m2[14] + m1[12] * m2[15];
        m_matrix[13] = m1[1] * m2[12] + m1[5] * m2[13] + m1[9] * m2[14] + m1[13] * m2[15];
        m_matrix[14] = m1[2] * m2[12] + m1[6] * m2[13] + m1[10] * m2[14] + m1[14] * m2[15];
        m_matrix[15] = m1[3] * m2[12] + m1[7] * m2[13] + m1[11] * m2[14] + m1[15] * m2[15];

        return *this;
    }

    template <class T>
    TMatrix4D<T> TMatrix4D<T>::operator*(const TMatrix4D<T>& other) const
    {
        TMatrix4D<T> temp;

        const T * m1 = m_matrix;
        const T * m2 = other.m_matrix;

        temp.m_matrix[0] = m1[0] * m2[0] + m1[4] * m2[1] + m1[8] * m2[2] + m1[12] * m2[3];
        temp.m_matrix[1] = m1[1] * m2[0] + m1[5] * m2[1] + m1[9] * m2[2] + m1[13] * m2[3];
        temp.m_matrix[2] = m1[2] * m2[0] + m1[6] * m2[1] + m1[10] * m2[2] + m1[14] * m2[3];
        temp.m_matrix[3] = m1[3] * m2[0] + m1[7] * m2[1] + m1[11] * m2[2] + m1[15] * m2[3];
        temp.m_matrix[4] = m1[0] * m2[4] + m1[4] * m2[5] + m1[8] * m2[6] + m1[12] * m2[7];
        temp.m_matrix[5] = m1[1] * m2[4] + m1[5] * m2[5] + m1[9] * m2[6] + m1[13] * m2[7];
        temp.m_matrix[6] = m1[2] * m2[4] + m1[6] * m2[5] + m1[10] * m2[6] + m1[14] * m2[7];
        temp.m_matrix[7] = m1[3] * m2[4] + m1[7] * m2[5] + m1[11] * m2[6] + m1[15] * m2[7];
        temp.m_matrix[8] = m1[0] * m2[8] + m1[4] * m2[9] + m1[8] * m2[10] + m1[12] * m2[11];
        temp.m_matrix[9] = m1[1] * m2[8] + m1[5] * m2[9] + m1[9] * m2[10] + m1[13] * m2[11];
        temp.m_matrix[10] = m1[2] * m2[8] + m1[6] * m2[9] + m1[10] * m2[10] + m1[14] * m2[11];
        temp.m_matrix[11] = m1[3] * m2[8] + m1[7] * m2[9] + m1[11] * m2[10] + m1[15] * m2[11];
        temp.m_matrix[12] = m1[0] * m2[12] + m1[4] * m2[13] + m1[8] * m2[14] + m1[12] * m2[15];
        temp.m_matrix[13] = m1[1] * m2[12] + m1[5] * m2[13] + m1[9] * m2[14] + m1[13] * m2[15];
        temp.m_matrix[14] = m1[2] * m2[12] + m1[6] * m2[13] + m1[10] * m2[14] + m1[14] * m2[15];
        temp.m_matrix[15] = m1[3] * m2[12] + m1[7] * m2[13] + m1[11] * m2[14] + m1[15] * m2[15];

        return temp;
    }

    template<class T>
    TVector4D<T> TMatrix4D<T>::operator*(const TVector4D<T>& vector) const
    {
        T x = m_matrix[0] * vector.x + m_matrix[4] * vector.y + m_matrix[8] * vector.z + m_matrix[12] * vector.w;
        T y = m_matrix[1] * vector.x + m_matrix[5] * vector.y + m_matrix[9] * vector.z + m_matrix[13] * vector.w;
        T z = m_matrix[2] * vector.x + m_matrix[6] * vector.y + m_matrix[10] * vector.z + m_matrix[14] * vector.w;
        T w = m_matrix[3] * vector.x + m_matrix[7] * vector.y + m_matrix[11] * vector.z + m_matrix[15] * vector.w;

        return TVector4D<T>(x, y, z, w);
    }

    template <class T>
    TVector3D<T> TMatrix4D<T>::getTranslation() const
    {
        return TVector3D<T>(m_matrix[12], m_matrix[13], m_matrix[14]);
    }

    template <class T>
    void TMatrix4D<T>::setTranslation(const TVector3D<T>& translation)
    {
        m_matrix[12] = translation.x;
        m_matrix[13] = translation.y;
        m_matrix[14] = translation.z;
    }


    template <class T>
    void TMatrix4D<T>::preScale(const TVector3D<T>& scale)
    {
        m_matrix[0] *= scale.x;
        m_matrix[1] *= scale.y;
        m_matrix[2] *= scale.z;

        m_matrix[4] *= scale.x;
        m_matrix[5] *= scale.y;
        m_matrix[6] *= scale.z;

        m_matrix[8] *= scale.x;
        m_matrix[9] *= scale.y;
        m_matrix[10] *= scale.z;

        m_matrix[12] *= scale.x;
        m_matrix[13] *= scale.y;
        m_matrix[14] *= scale.z;
    }

    template <class T>
    void TMatrix4D<T>::postScale(const TVector3D<T>& scale)
    {
        m_matrix[0] *= scale.x;
        m_matrix[1] *= scale.x;
        m_matrix[2] *= scale.x;

        m_matrix[4] *= scale.y;
        m_matrix[5] *= scale.y;
        m_matrix[6] *= scale.y;

        m_matrix[8] *= scale.z;
        m_matrix[9] *= scale.z;
        m_matrix[10] *= scale.z;
    }

    template <class T>
    void TMatrix4D<T>::setScale(const TVector3D<T>& scale)
    {
        m_matrix[0] = scale.x;
        m_matrix[5] = scale.y;
        m_matrix[10] = scale.z;
    }


    template <class T>
    TVector3D<T> TMatrix4D<T>::getScale() const
    {
        TVector3D<T> scale;
        scale.x = TVector3D<T>(m_matrix[0], m_matrix[1], m_matrix[2]).length();
        scale.y = TVector3D<T>(m_matrix[4], m_matrix[5], m_matrix[6]).length();
        scale.z = TVector3D<T>(m_matrix[8], m_matrix[9], m_matrix[10]).length();
        return scale;
    }

    template <class T>
    void TMatrix4D<T>::setRotation(const TVector3D<T>& rotation)
    {
        f64 rotX = (f64)rotation.x * k_degToRad;
        f64 rotY = (f64)rotation.y * k_degToRad;
        f64 rotZ = (f64)rotation.z * k_degToRad;

        const f64 cr = cos(rotX);
        const f64 sr = sin(rotX);
        const f64 cp = cos(rotY);
        const f64 sp = sin(rotY);
        const f64 cy = cos(rotZ);
        const f64 sy = sin(rotZ);

        m_matrix[0] = (T)(cp*cy);
        m_matrix[1] = (T)(cp*sy);
        m_matrix[2] = (T)(-sp);

        const f64 srsp = sr * sp;
        const f64 crsp = cr * sp;

        m_matrix[4] = (T)(srsp * cy - cr * sy);
        m_matrix[5] = (T)(srsp * sy + cr * cy);
        m_matrix[6] = (T)(sr * cp);

        m_matrix[8] = (T)(crsp * cy + sr * sy);
        m_matrix[9] = (T)(crsp * sy - sr * cy);
        m_matrix[10] = (T)(cr * cp);
    }

    template <class T>
    TVector3D<T> TMatrix4D<T>::getRotation() const
    {
        const TMatrix4D<T> &mat = *this;

        f64 Y = -asin(clamp((f64)mat(0, 2), -1.0, 1.0));
        const f64 C = cos(Y);
        Y *= k_radToDeg;

        f64 rotx, roty, X, Z;

        if (abs(C) > k_tolerance64)
        {
            const f64 invC = (f64)(1.0 / C);
            rotx = mat(2, 2) * invC;
            roty = mat(1, 2) * invC;
            X = atan2(roty, rotx) * k_radToDeg;
            rotx = mat(0, 0) * invC;
            roty = mat(0, 1) * invC;
            Z = atan2(roty, rotx) * k_radToDeg;
        }
        else
        {
            X = 0.0;
            rotx = mat(1, 1);
            roty = -mat(1, 0);
            Z = atan2(roty, rotx) * k_radToDeg;
        }

        if (X < 0.0) X += 360.0;
        if (Y < 0.0) Y += 360.0;
        if (Z < 0.0) Z += 360.0;

        return TVector3D<T>((T)X, (T)Y, (T)Z);
    }


    template <class T>
    void TMatrix4D<T>::makeIdentity()
    {
        m_matrix[1] = m_matrix[2] = m_matrix[3] = m_matrix[4] =
            m_matrix[6] = m_matrix[7] = m_matrix[8] = m_matrix[9] =
            m_matrix[11] = m_matrix[12] = m_matrix[13] = m_matrix[14] = (T)0;
        m_matrix[0] = m_matrix[5] = m_matrix[10] = m_matrix[15] = (T)1;
    }

    template <class T>
    bool TMatrix4D<T>::isIdentity() const
    {
        if (!isEquals(m_matrix[0], (T)1) ||
            !isEquals(m_matrix[5], (T)1) ||
            !isEquals(m_matrix[10], (T)1) ||
            !isEquals(m_matrix[15], (T)1))
        {
            return false;
        }

        for (s32 i = 0; i < 4; ++i)
        {
            for (s32 j = 0; j < 4; ++j)
            {
                if ((j != i) && (!isZero((*this)(i, j))))
                {
                    return false;
                }
            }
        }

        return true;
    }

    template <class T>
    bool TMatrix4D<T>::getInverse(TMatrix4D<T>& outMatrix) const
    {
        const T t0 = m_matrix[10] * m_matrix[15] - m_matrix[11] * m_matrix[14];
        const T t1 = m_matrix[6] * m_matrix[15] - m_matrix[7] * m_matrix[14];
        const T t2 = m_matrix[6] * m_matrix[11] - m_matrix[7] * m_matrix[10];

        const T t3 = m_matrix[2] * m_matrix[15] - m_matrix[3] * m_matrix[14];
        const T t4 = m_matrix[2] * m_matrix[11] - m_matrix[3] * m_matrix[10];
        const T t5 = m_matrix[2] * m_matrix[7] - m_matrix[3] * m_matrix[6];

        const T t6 = m_matrix[8] * m_matrix[13] - m_matrix[9] * m_matrix[12];
        const T t7 = m_matrix[4] * m_matrix[13] - m_matrix[5] * m_matrix[12];
        const T t8 = m_matrix[4] * m_matrix[9] - m_matrix[5] * m_matrix[8];

        const T t9 = m_matrix[0] * m_matrix[13] - m_matrix[1] * m_matrix[12];
        const T t10 = m_matrix[0] * m_matrix[9] - m_matrix[1] * m_matrix[8];
        const T t11 = m_matrix[0] * m_matrix[5] - m_matrix[1] * m_matrix[4];

        T det = t0 * t11 - t1 * t10 + t2 * t9 + t3 * t8 - t4 * t7 + t5 * t6;

        if (isZero(det))
        {
            return false;
        }

        det = (T)1.0 / det;

        const T ft0 = (T)(t0 * det);
        const T ft1 = (T)(t1 * det);
        const T ft2 = (T)(t2 * det);

        const T ft3 = (T)(t3 * det);
        const T ft4 = (T)(t4 * det);
        const T ft5 = (T)(t5 * det);

        const T ft6 = (T)(t6 * det);
        const T ft7 = (T)(t7 * det);
        const T ft8 = (T)(t8 * det);

        const T ft9 = (T)(t9 * det);
        const T ft10 = (T)(t10 * det);
        const T ft11 = (T)(t11 * det);

        outMatrix.m_matrix[0] = m_matrix[5] * ft0 - m_matrix[9] * ft1 + m_matrix[13] * ft2;
        outMatrix.m_matrix[1] = m_matrix[9] * ft3 - m_matrix[1] * ft0 - m_matrix[13] * ft4;
        outMatrix.m_matrix[2] = m_matrix[1] * ft1 - m_matrix[5] * ft3 + m_matrix[13] * ft5;
        outMatrix.m_matrix[3] = m_matrix[5] * ft4 - m_matrix[1] * ft2 - m_matrix[9] * ft5;

        outMatrix.m_matrix[4] = m_matrix[8] * ft1 - m_matrix[4] * ft0 - m_matrix[12] * ft2;
        outMatrix.m_matrix[5] = m_matrix[0] * ft0 - m_matrix[8] * ft3 + m_matrix[12] * ft4;
        outMatrix.m_matrix[6] = m_matrix[4] * ft3 - m_matrix[0] * ft1 - m_matrix[12] * ft5;
        outMatrix.m_matrix[7] = m_matrix[0] * ft2 - m_matrix[4] * ft4 + m_matrix[8] * ft5;

        outMatrix.m_matrix[8] = m_matrix[7] * ft6 - m_matrix[11] * ft7 + m_matrix[15] * ft8;
        outMatrix.m_matrix[9] = m_matrix[11] * ft9 - m_matrix[3] * ft6 - m_matrix[15] * ft10;
        outMatrix.m_matrix[10] = m_matrix[3] * ft7 - m_matrix[7] * ft9 + m_matrix[15] * ft11;
        outMatrix.m_matrix[11] = m_matrix[7] * ft10 - m_matrix[3] * ft8 - m_matrix[11] * ft11;

        outMatrix.m_matrix[12] = m_matrix[10] * ft7 - m_matrix[6] * ft6 - m_matrix[14] * ft8;
        outMatrix.m_matrix[13] = m_matrix[2] * ft6 - m_matrix[10] * ft9 + m_matrix[14] * ft10;
        outMatrix.m_matrix[14] = m_matrix[6] * ft9 - m_matrix[2] * ft7 - m_matrix[14] * ft11;
        outMatrix.m_matrix[15] = m_matrix[2] * ft8 - m_matrix[6] * ft10 + m_matrix[10] * ft11;

        return true;
    }

    template <class T>
    bool TMatrix4D<T>::makeInverse()
    {
        TMatrix4D<T> temp;

        if (getInverse(temp))
        {
            *this = temp;
            return true;
        }

        return false;
    }

    template <class T>
    TMatrix4D<T>& TMatrix4D<T>::operator=(const T& scalar)
    {
        for (s32 i = 0; i < 16; ++i)
        {
            m_matrix[i] = scalar;
        }
        return *this;
    }

    template <class T>
    bool TMatrix4D<T>::operator==(const TMatrix4D<T> &other) const
    {
        for (s32 i = 0; i < 16; ++i)
        {
            if (m_matrix[i] != other.m_matrix[i])
            {
                return false;
            }
        }
        return true;
    }

    template <class T>
    bool TMatrix4D<T>::operator!=(const TMatrix4D<T> &other) const
    {
        return !(*this == other);
    }

    template <class T>
    void TMatrix4D<T>::makeTransposed()
    {
        TMatrix4D<T> temp(*this);

        m_matrix[0] = temp.m_matrix[0];
        m_matrix[1] = temp.m_matrix[4];
        m_matrix[2] = temp.m_matrix[8];
        m_matrix[3] = temp.m_matrix[12];

        m_matrix[4] = temp.m_matrix[1];
        m_matrix[5] = temp.m_matrix[5];
        m_matrix[6] = temp.m_matrix[9];
        m_matrix[7] = temp.m_matrix[13];

        m_matrix[8] = temp.m_matrix[2];
        m_matrix[9] = temp.m_matrix[6];
        m_matrix[10] = temp.m_matrix[10];
        m_matrix[11] = temp.m_matrix[14];

        m_matrix[12] = temp.m_matrix[3];
        m_matrix[13] = temp.m_matrix[7];
        m_matrix[14] = temp.m_matrix[11];
        m_matrix[15] = temp.m_matrix[15];
    }

    template <class T>
    TMatrix4D<T> TMatrix4D<T>::getTransposed() const
    {
        TMatrix4D<T> out;

        out.m_matrix[0] = m_matrix[0];
        out.m_matrix[1] = m_matrix[4];
        out.m_matrix[2] = m_matrix[8];
        out.m_matrix[3] = m_matrix[12];

        out.m_matrix[4] = m_matrix[1];
        out.m_matrix[5] = m_matrix[5];
        out.m_matrix[6] = m_matrix[9];
        out.m_matrix[7] = m_matrix[13];

        out.m_matrix[8] = m_matrix[2];
        out.m_matrix[9] = m_matrix[6];
        out.m_matrix[10] = m_matrix[10];
        out.m_matrix[11] = m_matrix[14];

        out.m_matrix[12] = m_matrix[3];
        out.m_matrix[13] = m_matrix[7];
        out.m_matrix[14] = m_matrix[11];
        out.m_matrix[15] = m_matrix[15];

        return out;
    }

    template <class T>
    void TMatrix4D<T>::set(const T* matrix)
    {
        if (matrix)
            memcpy(m_matrix, matrix, 16 * sizeof(T));
    }

    template <class T>
    const T* TMatrix4D<T>::getPtr() const
    {
        return m_matrix;
    }

    template <class T>
    T* TMatrix4D<T>::getPtr()
    {
        return m_matrix;
    }

} //namespace core
} //namespace v3d
