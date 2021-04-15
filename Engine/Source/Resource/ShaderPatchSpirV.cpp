#include "ShaderPatchSpirV.h"

#include "Utils/Timer.h"
#include "Utils/Logger.h"

#include "Renderer/Context.h"
#ifdef USE_SPIRV
# include <shaderc/third_party/glslang/SPIRV/spirv.hpp>

namespace v3d
{
namespace resource
{

/////////////////////////////////////////////////////////////////////////////////////////////////////

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

ShaderPatcherSpirV::ShaderPatcherSpirV(u32 flags) noexcept
    : m_flags(flags)
{
}

bool ShaderPatcherSpirV::process(PatchSpirvCode* patch, std::vector<u32>& spirv)
{
    if (spirv.empty())
    {
        return false;
    }

    auto word = spirv.begin();

    u32 magicNumber = *word;
    if (magicNumber != 0x07230203)
    {
        return false;
    }

    [[maybe_unused]] u32 versionNumber = *std::next(word, 1);
    [[maybe_unused]] u32 generator = *std::next(word, 2);
    [[maybe_unused]] u32 boundIDs = *std::next(word, 3);

#if DEBUG
    utils::Timer timer;
    timer.start();
#endif

    std::vector<u32> patchedSpirv(spirv);
    bool result = patch->patch(spirv, patchedSpirv, m_flags);
    if (result)
    {
        std::swap(spirv, patchedSpirv);
    }

#if DEBUG
    timer.stop();
    u64 time = timer.getTime<utils::Timer::Duration_MilliSeconds>();
    LOG_DEBUG("ShaderPatcherSpirV::process, spirv patch time %.4f sec", static_cast<f32>(time) / 1000.0f);
#endif

    LOG_DEBUG("ShaderPatcherSpirV::process result %d", result);
    return result;
}

} //namespace resource
} //namespace v3d
#endif //USE_SPIRV