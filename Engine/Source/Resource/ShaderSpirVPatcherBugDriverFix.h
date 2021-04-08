#pragma once

#include "Common.h"

#ifdef USE_SPIRV
#include "ShaderPatchSpirV.h"

namespace v3d
{
namespace resource
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class PatchDriverBugOptimization final : public PatchSpirvCode
    {
    public:

        PatchDriverBugOptimization() noexcept = default;
        ~PatchDriverBugOptimization() = default;

        bool patch(const std::vector<u32>& spirv, std::vector<u32>& patchedSpirv, u32 flags = 0) override;

    private:

        bool composePatch(std::function<bool(std::vector<u32>& subSpirv)> func, std::vector<u32>& subSpirv);
        [[maybe_unused]] u32 m_flags;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
#endif //USE_SPIRV
