#pragma once

#include "Math.h"
#include "Vector3D.h"
#include "VectorRegister.h"

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

        void expand(const TVector3D<T>& value);

        [[nodiscard]] bool isInside(const TVector3D<T>& point) const;
        [[nodiscard]] bool isValid() const;

    private:

        TVector3D<T> _min;
        TVector3D<T> _max;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    using AABB = TMinMaxAABB<f32>;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<typename T>
    inline bool calculateVerticesFromAABB(const AABB& aabb, TVector3D<T> vertices[8])
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
        vertices[0].set(aabb.getMin()._x, aabb.getMin()._y, aabb.getMin()._z);
        vertices[1].set(aabb.getMax()._x, aabb.getMin()._y, aabb.getMin()._z);
        vertices[2].set(aabb.getMax()._x, aabb.getMax()._y, aabb.getMin()._z);
        vertices[3].set(aabb.getMin()._x, aabb.getMax()._y, aabb.getMin()._z);
        vertices[4].set(aabb.getMin()._x, aabb.getMin()._y, aabb.getMax()._z);
        vertices[5].set(aabb.getMax()._x, aabb.getMin()._y, aabb.getMax()._z);
        vertices[6].set(aabb.getMax()._x, aabb.getMax()._y, aabb.getMax()._z);
        vertices[7].set(aabb.getMin()._x, aabb.getMax()._y, aabb.getMax()._z);

        return true;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace math
} //namespace v3d

#include "AABB.inl"
