#pragma once

#include "ResourceDecoder.h"
#include "Renderer/Shader.h"

#ifdef USE_SPIRV
namespace v3d
{
namespace resource
{
    class Resource;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief ShaderSpirVDecoder decoder.
    * Supports source formats: "vert", "frag", "ps", "vs", "hlsl"
    * Supports binary formats: "vspv", "fspv"
    * @see https://github.com/google/shaderc
    */
    class ShaderSpirVDecoder final : public ResourceDecoder
    {
    public:

        explicit ShaderSpirVDecoder(const renderer::ShaderHeader& header, const std::string& entrypoint, const renderer::Shader::DefineList& defines,
            const std::vector<std::string>& includes, renderer::ShaderCompileFlags flags = 0) noexcept;

        explicit ShaderSpirVDecoder(std::vector<std::string> supportedExtensions, const renderer::ShaderHeader& header, const std::string& entrypoint, const renderer::Shader::DefineList& defines,
            const std::vector<std::string>& includes, renderer::ShaderCompileFlags flags = 0) noexcept;

        ~ShaderSpirVDecoder() = default;

        [[nodiscard]] Resource* decode(const stream::Stream* stream, const std::string& name = "") const override;

    private:

        const renderer::ShaderHeader m_header;
        const bool m_reflections;

        const std::string m_entrypoint;
        const renderer::Shader::DefineList m_defines;
        const std::vector<std::string> m_includes;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
#endif //USE_SPIRV