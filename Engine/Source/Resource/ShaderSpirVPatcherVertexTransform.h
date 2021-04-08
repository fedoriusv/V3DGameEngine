#pragma once

#include "Common.h"

#ifdef USE_SPIRV
#include "ShaderPatchSpirV.h"

namespace v3d
{
namespace resource
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class PatchVertexTransform final : public PatchSpirvCode
    {
    public:

        explicit PatchVertexTransform(f32 angle) noexcept;
        ~PatchVertexTransform() = default;

        bool patch(std::vector<u32>& spirv, u32 flags = 0) override;

    private:

        f32 m_angle;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////


} //namespace resource
} //namespace v3d
#endif //USE_SPIRV
