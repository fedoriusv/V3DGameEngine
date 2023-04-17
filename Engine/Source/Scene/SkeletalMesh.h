#pragma once

#include "Common.h"
#include "StaticMesh.h"

namespace v3d
{
namespace scene
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief SkeletalMeshHeader meta info of StaticMesh
    */
    struct SkeletalMeshHeader : MeshHeader
    {

    };

    class SkeletalMesh : public StaticMesh
    {
    public:

        explicit SkeletalMesh(const SkeletalMeshHeader* header) noexcept;
        ~SkeletalMesh() noexcept;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
} //namespace scene
} //namespace v3d