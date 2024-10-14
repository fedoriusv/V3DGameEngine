#pragma once

#include "ShaderDecoder.h"

#ifdef USE_SPIRV
namespace v3d
{
namespace resource
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    class Resource;

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

        explicit ShaderSpirVDecoder() noexcept;
        explicit ShaderSpirVDecoder(const std::vector<std::string>& supportedExtensions) noexcept;
        explicit ShaderSpirVDecoder(std::vector<std::string>&& supportedExtensions) noexcept;

        ~ShaderSpirVDecoder() = default;

        [[nodiscard]] Resource* decode(const stream::Stream* stream, const Policy* policy, u32 flags = 0, const std::string& name = "") const override;

    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
#endif //USE_SPIRV