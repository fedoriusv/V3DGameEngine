#pragma once

#include "ResourceDecoder.h"
#include "Renderer/Shader.h"

#if defined(PLATFORM_WINDOWS) || defined(PLATFORM_XBOX)

struct IDxcBlob;

namespace v3d
{
namespace resource
{
    class Resource;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief ShaderDXCDecoder decoder.
    * Support source formats: "vs", "ps", "cs", "hlsl".
    * Support shader model 6.0 and above
    * @see https://github.com/microsoft/DirectXShaderCompiler
    */
    class ShaderDXCDecoder final : public ResourceDecoder
    {
    public:

        explicit ShaderDXCDecoder(const renderer::ShaderHeader& header, const std::string& entrypoint, const renderer::Shader::DefineList& defines,
            const std::vector<std::string>& includes, renderer::ShaderHeader::ShaderModel output, renderer::ShaderCompileFlags flags = 0) noexcept;

        explicit ShaderDXCDecoder(std::vector<std::string> supportedExtensions, const renderer::ShaderHeader& header, const std::string& entrypoint, const renderer::Shader::DefineList& defines,
            const std::vector<std::string>& includes, renderer::ShaderHeader::ShaderModel output, renderer::ShaderCompileFlags flags = 0) noexcept;

        ~ShaderDXCDecoder() = default;

        [[nodiscard]] Resource* decode(const stream::Stream* stream, const std::string& name = "") const override;

    private:

        bool compile(const std::string& source, renderer::ShaderType shaderType, const std::wstring& name, IDxcBlob*& shader) const;
        bool reflect(stream::Stream* stream, IDxcBlob* shader) const;

        const renderer::ShaderHeader m_header;
        const bool m_reflections;

        const std::string m_entrypoint;
        const renderer::Shader::DefineList m_defines;
        const std::vector<std::string> m_includes;

        renderer::ShaderHeader::ShaderModel m_outputSM;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
#endif //PLATFORM