#pragma once

#include "Common.h"

#ifdef USE_SPIRV
#include "ShaderPatchSpirV.h"

namespace v3d
{
namespace resource
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class PatchInvertOrdinate final : public PatchSpirvCode
    {
    public:

        PatchInvertOrdinate() = default;
        ~PatchInvertOrdinate() = default;

        bool patch(const std::vector<u32>& spirv, std::vector<u32>& patchedSpirv, u32 flags = 0) override;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////


} //namespace resource
} //namespace v3d
#endif //USE_SPIRV
