#include "ShaderPatchSpirV.h"
#include "Renderer/Context.h"

#ifdef USE_SPIRV
#   include <shaderc/third_party/glslang/SPIRV/spirv.hpp>

namespace v3d
{
namespace resource
{

u32 getOpCode(u32 word)
{
    return word & 0xFFFF;
}

u32 getCountWords(u32 word)
{
    return word >> 16;
}

u32 getWordInstruction(u32 op, u32 count)
{
    return count << 16 | op;
}

[[maybe_unused]] std::string getLiteralString(const u32& word, u32& byteSize)
{
    const c8* ch = reinterpret_cast<const c8*>(&word);
    std::string str(ch);

    u32 countBytes = static_cast<u32>(str.size()) + 1; //'\0'
    byteSize = static_cast<u32>(ceilf(static_cast<f32>(countBytes) / static_cast<f32>(sizeof(u32)))) - 1;

    return str;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderPatcherSpirV::process(PatchSpirvCode* patch, const std::vector<u32>& spirvIn, std::vector<u32>& spirvOut)
{
    if (spirvIn.empty())
    {
        return false;
    }

    auto word = spirvIn.begin();

    u32 magicNumber = *word;
    if (magicNumber != 0x07230203)
    {
        return false;
    }

    [[maybe_unused]] u32 versionNumber = *std::next(word, 1);
    [[maybe_unused]] u32 generator = *std::next(word, 2);
    [[maybe_unused]] u32 boundIDs = *std::next(word, 3);

    spirvOut = spirvIn;
    return patch->patch(spirvOut);
}

} //namespace resource
} //namespace v3d
#endif //USE_SPIRV