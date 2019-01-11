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

        struct InputAttribute
        {
            InputAttribute()
                : _location(0)
                , _binding(0)
                , _offset(0)
                //, _type()
            {
            }

            u32         _location;
            u32         _binding;
            u32         _offset;
            //DataType    _type;

            void operator >> (stream::Stream * stream)
            {
                stream->write<u32>(_location);
            }

            void operator << (const stream::Stream * stream)
            {
                stream->read<u32>(_location);
            }


        };

        struct InputBinding
        {
            u32     _binding;
            u32     _stride;
            bool    _instance;
        };

        struct ReflectionInfo
        {
            std::vector<InputAttribute> _inputAttribute;
            std::vector<InputBinding>   _inputBinding;
        };

        Shader() noexcept;
        ~Shader();

        void init(const stream::Stream* stream, const ResourceHeader* header) override;
        bool load() override;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
