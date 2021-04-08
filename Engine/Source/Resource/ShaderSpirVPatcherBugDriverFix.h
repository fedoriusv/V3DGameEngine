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

        explicit PatchDriverBugOptimization(u32 flags = 0) noexcept;
        ~PatchDriverBugOptimization() = default;

        bool patch(std::vector<u32>& spirv, u32 flags = 0) override;

    private:

        bool composePatch(std::function<bool(std::vector<u32>& subSpirv)> func, std::vector<u32>& subSpirv);
        [[maybe_unused]] u32 m_flags;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
#endif //USE_SPIRV
