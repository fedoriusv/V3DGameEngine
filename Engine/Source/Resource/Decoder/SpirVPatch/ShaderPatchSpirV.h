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
    [[maybe_unused]] std::string getLiteralString(const u32& word, u32& byteSize);

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * PatchSpirvCode class. Patch binary shader
    */
    class PatchSpirvCode
    {
    public:

        PatchSpirvCode() noexcept = default;
        virtual ~PatchSpirvCode() = default;

        virtual bool patch(const std::vector<u32>& spirv, std::vector<u32>& patchedSpirv, u32 flags = 0) = 0;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * ShaderPatcherSpirV class. Patcher
    */
    class ShaderPatcherSpirV
    {
    public:

        explicit ShaderPatcherSpirV(u32 flags = 0) noexcept;
        ~ShaderPatcherSpirV() = default;

        bool process(PatchSpirvCode* patch, std::vector<u32>& spirv);

    private:

        u32 m_flags;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
#endif //USE_SPIRV
