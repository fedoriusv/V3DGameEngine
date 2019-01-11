#pragma once

#include "Resource.h"

namespace v3d
{
namespace resource
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * ShaderHeader meta info about Shader
    */
    struct ShaderHeader : ResourceHeader
    {
        enum class ShaderResource : u32
        {
            ShaderResource_Source,
            ShaderResource_SpirVBytecode,
        };

        enum class ShaderLang : u32
        {
            ShaderLang_GLSL,
            ShaderLang_HLSL,
            ShaderLang_CG
        };

        ShaderResource _contentType;
        ShaderLang     _shaderLang;
        f32            _apiVersion;
        u32            _optLevel;

        std::vector<std::pair<std::string, std::string>> _defines;
        std::vector<std::string>                         _include;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Shader class
    */
    class Shader : public Resource
    {
    public:

        Shader();
        ~Shader();

        void init(const stream::Stream* stream, const ResourceHeader* header) override;
        bool load() override;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
