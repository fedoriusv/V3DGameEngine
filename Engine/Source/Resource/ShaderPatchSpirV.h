#pragma once

#include "Common.h"

#ifdef USE_SPIRV
namespace v3d
{
namespace resource
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    u32 getOpCode(u32 word);
    u32 getCountWords(u32 word);
    u32 getWordInstruction(u32 op, u32 count);
    [[maybe_unused]] std::string getLiteralString(u32 word, u32& byteSize);

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

} //namespace resource
} //namespace v3d
#endif //USE_SPIRV
