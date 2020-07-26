namespace v3d
{
namespace core
{

    template <class T>
    TMatrix2D<T>::TMatrix2D()
    {
        m_matrix[1] = m_matrix[2] = (T)0;
        m_matrix[0] = m_matrix[3] = (T)1;
    }

    template <class T>
    TMatrix2D<T>::TMatrix2D(const TMatrix2D<T>& other)
    {
        *this = other;
    }

    template <class T>
    TMatrix2D<T>::TMatrix2D(const T* matrix)
    {
        memcpy(m_matrix, matrix, 4 * sizeof(T));
    }

    template <class T>
    TMatrix2D<T>::TMatrix2D(const T& m0, const T& m1,
        const T& m2, const T& m3)
    {
        m_matrix[0] = m0;
        m_matrix[1] = m1;
        m_matrix[2] = m2;
        m_matrix[3] = m3;
    }

    template <class T>
    T& TMatrix2D<T>::operator()(u32 row, u32 col)
    {
        return m_matrix[row * 2 + col];
    }

    template <class T>
    const T& TMatrix2D<T>::operator()(u32 row, u32 col) const
    {
        return m_matrix[row * 2 + col];
    }

    template <class T>
    T& TMatrix2D<T>::operator[](u32 index)
    {
        return m_matrix[index];
    }

    template <class T>
    const T& TMatrix2D<T>::operator[](u32 index) const
    {
        return m_matrix[index];
    }

    template <class T>
    TMatrix2D<T>& TMatrix2D<T>::operator=(const T& scalar)
    {
        for (s32 i = 0; i < 4; ++i)
        {
            m_matrix[i] = scalar;
        }
        return *this;
    }

    template <class T>
    const T* TMatrix2D<T>::getPtr() const
    {
        return m_matrix;
    }

    template <class T>
    T* TMatrix2D<T>::getPtr()
    {
        return m_matrix;
    }

    template <class T>
    bool TMatrix2D<T>::operator==(const TMatrix2D<T> &other) const
    {
        for (s32 i = 0; i < 4; ++i)
        {
            if (m_matrix[i] != other.m_matrix[i])
            {
                return false;
            }
        }
        return true;
    }

    template <class T>
    bool TMatrix2D<T>::operator!=(const TMatrix2D<T> &other) const
    {
        return !(*this == other);
    }

    template <class T>
    TMatrix2D<T> TMatrix2D<T>::operator+(const TMatrix2D<T>& other) const
    {
        TMatrix2D<T> temp;

        temp[0] = m_matrix[0] + other[0];
        temp[1] = m_matrix[1] + other[1];
        temp[2] = m_matrix[2] + other[2];
        temp[3] = m_matrix[3] + other[3];

        return temp;
    }

    template <class T>
    TMatrix2D<T>& TMatrix2D<T>::operator+=(const TMatrix2D<T>& other)
    {
        m_matrix[0] += other[0];
        m_matrix[1] += other[1];
        m_matrix[2] += other[2];
        m_matrix[3] += other[3];

        return *this;
    }

    template <class T>
    TMatrix2D<T> TMatrix2D<T>::operator-(const TMatrix2D<T>& other) const
    {
        TMatrix2D<T> temp;

        temp[0] = m_matrix[0] - other[0];
        temp[1] = m_matrix[1] - other[1];
        temp[2] = m_matrix[2] - other[2];
        temp[3] = m_matrix[3] - other[3];

        return temp;
    }

    template <class T>
    TMatrix2D<T>& TMatrix2D<T>::operator-=(const TMatrix2D<T>& other)
    {
        m_matrix[0] -= other[0];
        m_matrix[1] -= other[1];
        m_matrix[2] -= other[2];
        m_matrix[3] -= other[3];

        return *this;
    }
    template <class T>
    TMatrix2D<T> TMatrix2D<T>::operator*(const TMatrix2D<T>& other) const
    {
        TMatrix2D<T> temp;

        const T* m1 = m_matrix;
        const T* m2 = other.m_matrix;

        temp.m_matrix[0] = m1[0] * m2[0] + m1[2] * m2[1];
        temp.m_matrix[1] = m1[1] * m2[0] + m1[3] * m2[1];
        temp.m_matrix[2] = m1[0] * m2[2] + m1[2] * m2[3];
        temp.m_matrix[3] = m1[1] * m2[2] + m1[3] * m2[3];

        return temp;
    }

    template <class T>
    TMatrix2D<T>& TMatrix2D<T>::operator*=(const TMatrix2D<T>& other)
    {
        TMatrix2D<T> temp(*this);

        const T * m1 = temp.m_matrix;
        const T * m2 = other.m_matrix;

        m_matrix[0] = m1[0] * m2[0] + m1[2] * m2[1];
        m_matrix[1] = m1[1] * m2[0] + m1[3] * m2[1];
        m_matrix[2] = m1[0] * m2[2] + m1[2] * m2[3];
        m_matrix[3] = m1[1] * m2[2] + m1[3] * m2[3];

        return *this;
    }

    template <class T>
    TMatrix2D<T> TMatrix2D<T>::operator*(const T& scalar) const
    {
        TMatrix2D<T> temp;

        temp[0] = m_matrix[0] * scalar;
        temp[1] = m_matrix[1] * scalar;
        temp[2] = m_matrix[2] * scalar;
        temp[3] = m_matrix[3] * scalar;

        return temp;
    }

    template <class T>
    TMatrix2D<T>& TMatrix2D<T>::operator*=(const T& scalar)
    {
        m_matrix[0] *= scalar;
        m_matrix[1] *= scalar;
        m_matrix[2] *= scalar;
        m_matrix[3] *= scalar;

        return *this;
    }

    template <class T>
    void TMatrix2D<T>::makeIdentity()
    {
        m_matrix[1] = m_matrix[2] = (T)0;
        m_matrix[0] = m_matrix[3] = (T)1;
    }

    template <class T>
    bool TMatrix2D<T>::isIdentity() const
    {
        if (!isEquals(m_matrix[0], (T)1) || !isEquals(m_matrix[3], (T)1))
        {
            return false;
        }
        if (!isZero(m_matrix[1]) || !isZero(m_matrix[2]))
        {
            return false;
        }
        return true;
    }

    template <class T>
    void TMatrix2D<T>::setRotation(T angle)
    {
        const T cy = cos(angle);
        const T sy = sin(angle);

        m_matrix[0] = (T)(cy);
        m_matrix[1] = (T)(-sy);

        m_matrix[2] = (T)(sy);
        m_matrix[3] = (T)(cy);
    }

    template <class T>
    T TMatrix2D<T>::getRotation() const
    {
        const TMatrix2D<T> &mat = *this;

        f64 sin = clamp((f64)mat(1, 0), -1.0, 1.0);
        f64 cos = clamp((f64)mat(0, 0), -1.0, 1.0);
        T angle = (T)atan2(sin, cos);

        if (angle < 0.0)
        {
            angle += k_2pi;
        }

        return angle;
    }

    template <class T>
    inline void TMatrix2D<T>::preScale(const TVector2D<T>& scale)
    {
        m_matrix[0] *= scale.x;
        m_matrix[1] *= scale.y;
        m_matrix[2] *= scale.x;
        m_matrix[3] *= scale.y;
    }

    template <class T>
    inline void TMatrix2D<T>::postScale(const TVector2D<T>& scale)
    {
        m_matrix[0] *= scale.x;
        m_matrix[1] *= scale.x;
        m_matrix[2] *= scale.y;
        m_matrix[3] *= scale.y;
    }

    template <class T>
    void TMatrix2D<T>::setScale(const TVector2D<T>& scale)
    {
        m_matrix[0] = scale.x;
        m_matrix[3] = scale.y;
    }

    template <class T>
    TVector2D<T> TMatrix2D<T>::getScale() const
    {
        TVector2D<T> scale;
        scale.x = TVector2D<T>(m_matrix[0], m_matrix[1]).length();
        scale.y = TVector2D<T>(m_matrix[2], m_matrix[3]).length();
        return scale;
    }

    template <class T>
    bool TMatrix2D<T>::makeInverse()
    {
        TMatrix2D<T> temp;

        if (getInverse(temp))
        {
            *this = temp;
            return true;
        }

        return false;
    }

    template <class T>
    bool TMatrix2D<T>::getInverse(TMatrix2D<T>& out) const
    {
        T det = m_matrix[0] * m_matrix[3] - m_matrix[1] * m_matrix[2];

        if (isZero(det))
        {
            return false;
        }

        det = (T)1.0 / det;

        out.m_matrix[0] = det * m_matrix[3];
        out.m_matrix[1] = -det * m_matrix[1];
        out.m_matrix[2] = -det * m_matrix[2];
        out.m_matrix[3] = det * m_matrix[0];

        return true;
    }

    template <class T>
    void TMatrix2D<T>::makeTransposed()
    {
        TMatrix2D<T> temp(*this);

        m_matrix[0] = temp.m_matrix[0];
        m_matrix[1] = temp.m_matrix[2];
        m_matrix[2] = temp.m_matrix[1];
        m_matrix[3] = temp.m_matrix[3];
    }

    template <class T>
    TMatrix2D<T> TMatrix2D<T>::getTransposed() const
    {
        TMatrix2D<T> temp;

        temp.m_matrix[0] = m_matrix[0];
        temp.m_matrix[1] = m_matrix[2];
        temp.m_matrix[2] = m_matrix[1];
        temp.m_matrix[3] = m_matrix[3];

        return temp;
    }

    template <class T>
    TMatrix2D<T>& TMatrix2D<T>::set(const T* data)
    {
        if (data)
        {
            memcpy(m_matrix, data, 4 * sizeof(T));
        }

        return *this;
    }

} //namespace core
} //namespace v3d
