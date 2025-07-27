#pragma once

#include "Common.h"
#include "Resource/Resource.h"
#include "Mesh.h"

namespace v3d
{
namespace scene
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief StaticMesh class
    */
    class StaticMesh : public Mesh
    {
    public:

        StaticMesh() noexcept;
        ~StaticMesh();

        TypePtr getType() const final;
    };

    inline TypePtr StaticMesh::getType() const
    {
        return typeOf<StaticMesh>();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    template<>
    struct TypeOf<scene::StaticMesh>
    {
        static TypePtr get()
        {
            static TypePtr ptr = nullptr;
            return (TypePtr)&ptr;
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace v3d
