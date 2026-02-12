#pragma once

#include "ShaderDecoder.h"

#ifdef USE_SPIRV
namespace v3d
{
namespace resource
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief ShaderSpirVDecoder decoder.
    * Supports source formats: "vert", "frag", "ps", "vs", "hlsl"
    * Supports binary formats: "vspv", "fspv"
    * @see https://github.com/google/shaderc
    */
    class ShaderSpirVDecoder final : public ShaderDecoder
    {
    public:

        explicit ShaderSpirVDecoder(ShaderCompileFlags compileFlags) noexcept;
        explicit ShaderSpirVDecoder(const std::vector<std::string>& supportedExtensions, ShaderCompileFlags compileFlags) noexcept;
        explicit ShaderSpirVDecoder(std::vector<std::string>&& supportedExtensions, ShaderCompileFlags compileFlags) noexcept;

        ~ShaderSpirVDecoder() = default;

        [[nodiscard]] Resource* decode(const stream::Stream* stream, const resource::Resource::LoadPolicy* policy, u32 flags = 0, const std::string& name = "") const override;

    private:

        ShaderCompileFlags m_compileFlags;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
#endif //USE_SPIRV