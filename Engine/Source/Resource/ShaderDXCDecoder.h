#pragma once

#include "ResourceDecoder.h"
#include "Renderer/Shader.h"

#ifdef D3D_RENDER

struct IDxcBlob;

namespace v3d
{
namespace resource
{
    class Resource;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief ShaderDXCDecoder decoder.
    * @see https://github.com/microsoft/DirectXShaderCompiler
    */
    class ShaderDXCDecoder final : public ResourceDecoder
    {
    public:

        ShaderDXCDecoder(const renderer::ShaderHeader& header, renderer::ShaderHeader::ShaderModel output, bool reflections = true) noexcept;
        ShaderDXCDecoder(std::vector<std::string> supportedExtensions, const renderer::ShaderHeader& header, renderer::ShaderHeader::ShaderModel output, bool reflections = true) noexcept;

        ~ShaderDXCDecoder() = default;

        Resource* decode(const stream::Stream* stream, const std::string& name = "") const override;

    private:

        bool compile(const std::string& source, const std::wstring& name, IDxcBlob*& shader) const;
        bool reflect(stream::Stream* stream, IDxcBlob* shader) const;

        const renderer::ShaderHeader m_header;
        const bool m_reflections;

        const renderer::ShaderHeader::ShaderModel m_output;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
#endif //D3D_RENDER