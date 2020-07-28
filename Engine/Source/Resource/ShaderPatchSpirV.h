#pragma once

#include "Common.h"

#define PATCH_SYSTEM 0

#ifdef USE_SPIRV
namespace v3d
{
namespace resource
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * PatchSpirvCode class. Patch binary shader
    */
    class PatchSpirvCode
    {
    public:

        PatchSpirvCode() noexcept = default;
        virtual ~PatchSpirvCode() = default;

        virtual bool patch(std::vector<u32>& spirv, u32 flags = 0) = 0;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * ShaderPatcherSpirV class. Patcher
    */
    class ShaderPatcherSpirV final
    {
    public:

        ShaderPatcherSpirV() noexcept = default;
        ~ShaderPatcherSpirV() = default;

        bool process(PatchSpirvCode* patch, const std::vector<u32>& spirvIn, std::vector<u32>& spirvOut);
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////


    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class PatchDriverBugOptimisation final : public PatchSpirvCode
    {
    public:

        explicit PatchDriverBugOptimisation(u32 flags = 0) noexcept;
        ~PatchDriverBugOptimisation() = default;

        bool patch(std::vector<u32>& spirv, u32 flags = 0) override;

    private:

        bool composePatch(std::function<bool(std::vector<u32>& subSpirv)> func, std::vector<u32>& subSpirv);
        u32 m_flags;
    };


    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
#endif //USE_SPIRV
