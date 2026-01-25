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
    * @brief StaticMesh class.
    * @see Mesh
    */
    class StaticMesh : public ComponentBase<StaticMesh, Mesh>
    {
    public:

        explicit StaticMesh(renderer::Device* device) noexcept;
        explicit StaticMesh(renderer::Device* device, const MeshHeader& header) noexcept;


    protected:

        virtual ~StaticMesh();

    private:

        bool load(const stream::Stream* stream, u32 offset = 0) override;
        bool save(stream::Stream* stream, u32 offset = 0) const override;
    };

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
