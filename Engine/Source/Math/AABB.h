#pragma once

#include "Math.h"
#include "Vector3D.h"

namespace v3d
{
namespace math
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief TMinMaxAABB class
    */
    template <class T>
    class TMinMaxAABB final
    {
    public:

        TMinMaxAABB() noexcept;
        TMinMaxAABB(const TMinMaxAABB& other) noexcept;
        TMinMaxAABB(const TVector3D<T>& min, const TVector3D<T>& max) noexcept;
        ~TMinMaxAABB() noexcept = default;

        [[nodiscard]] TMinMaxAABB& operator=(const TMinMaxAABB& other) const;

        T* getPtr();
        const T* getPtr() const;

        bool operator==(const TMinMaxAABB& bbox) const;
        bool operator!=(const TMinMaxAABB& bbox) const;

        const TVector3D<T>& getMin() const;
        const TVector3D<T>& getMax() const;

        [[nodiscard]] TVector3D<T> getCenter() const;
        [[nodiscard]] TVector3D<T> getExtent() const;
        [[nodiscard]] TVector3D<T> getSize() const;

        void expand(T value);
        void expand(const TVector3D<T>& value);

        [[nodiscard]] bool isInside(const TVector3D<T>& point) const;

        [[nodiscard]] bool isValid() const;

    private:

        TVector3D<T> m_min;
        TVector3D<T> m_max;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    using AABB = TMinMaxAABB<f32>;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    inline bool calculateVerticesFromAABB(const AABB& aabb, Vector3D vertices[8])
    {
        if (!aabb.isValid())
        {
            return false;
        }

        //    7-----6
        //   /|    /|
        //  3-----2 |
        //  | 4---|-5
        //  |/    |/
        //  0-----1
        vertices[0].set(aabb.getMin().m_x, aabb.getMin().m_y, aabb.getMin().m_z);
        vertices[1].set(aabb.getMax().m_x, aabb.getMin().m_y, aabb.getMin().m_z);
        vertices[2].set(aabb.getMax().m_x, aabb.getMax().m_y, aabb.getMin().m_z);
        vertices[3].set(aabb.getMin().m_x, aabb.getMax().m_y, aabb.getMin().m_z);
        vertices[4].set(aabb.getMin().m_x, aabb.getMin().m_y, aabb.getMax().m_z);
        vertices[5].set(aabb.getMax().m_x, aabb.getMin().m_y, aabb.getMax().m_z);
        vertices[6].set(aabb.getMax().m_x, aabb.getMax().m_y, aabb.getMax().m_z);
        vertices[7].set(aabb.getMin().m_x, aabb.getMax().m_y, aabb.getMax().m_z);

        return true;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace math
} //namespace v3d

#include "AABB.inl"
