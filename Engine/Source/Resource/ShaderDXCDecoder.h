#pragma once

#include "ShaderDecoder.h"
#include "Renderer/Shader.h"

#if defined(PLATFORM_WINDOWS) || defined(PLATFORM_XBOX)

struct IDxcBlob;

namespace v3d
{
namespace resource
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    class Resource;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief ShaderDXCDecoder decoder.
    * Support source formats: "vs", "ps", "cs", "hlsl".
    * Support shader model 6.0 and above
    * @see https://github.com/microsoft/DirectXShaderCompiler
    */
    class ShaderDXCDecoder final : public ShaderDecoder
    {
    public:

        explicit ShaderDXCDecoder() noexcept;
        explicit ShaderDXCDecoder(const std::vector<std::string>& supportedExtensions) noexcept;
        explicit ShaderDXCDecoder(std::vector<std::string>&& supportedExtensions) noexcept;

        ~ShaderDXCDecoder() = default;

        [[nodiscard]] Resource* decode(const stream::Stream* stream, const Policy* policy, u32 flags = 0, const std::string& name = "") const override;

    private:

        bool compile(const std::string& source, const ShaderPolicy* policy, ShaderCompileFlags flags, IDxcBlob*& shader, const std::string& name = "") const;
        bool reflect(stream::Stream* stream, const ShaderPolicy* policy, ShaderCompileFlags flags, IDxcBlob* shader, const std::string& name = "") const;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
#endif //PLATFORM