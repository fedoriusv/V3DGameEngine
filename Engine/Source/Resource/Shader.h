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
        enum class SpirVResource : u32
        {
            SpirVResource_Source,
            SpirVResource_Bytecode
        };

        enum class ShaderType : u32
        {
            ShaderType_Vertex,
            ShaderType_Fragment,
        };

        enum class ShaderLang : u32
        {
            ShaderLang_GLSL,
            ShaderLang_HLSL,
            ShaderLang_CG
        };

        SpirVResource  _contentType;
        ShaderType     _shaderType;
        ShaderLang     _shaderLang;
        f32            _apiVersion;
        u32            _optLevel;

        std::vector<std::pair<std::string, std::string>> _defines;
        std::vector<std::string>                         _include;
    };

    /**
    * Shader class
    */
    class Shader : public Resource
    {
    public:

        Shader() {};
        virtual ~Shader() {};

    private: 

        Shader(const ResourceHeader* header, stream::Stream* stream)
            : Resource(header, stream)
        {
        }

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
