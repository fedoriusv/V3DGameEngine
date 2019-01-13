#pragma once

#include "Resource.h"
#include "Renderer/ImageFormats.h"

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

        struct Attribute
        {
            Attribute();

            u32                 _location;
            //u32                 _binding;
            //u32                 _offset;
            renderer::Format    _format;
            std::string         _name;

            void operator >> (stream::Stream * stream);
            void operator << (const stream::Stream * stream);
        };

        /*struct InputBinding
        {
            u32     _binding;
            u32     _stride;
            bool    _instance;
        };*/

        struct ReflectionInfo
        {
            //std::vector<InputBinding>   _inputBinding;
            std::vector<Attribute> _inputAttribute;
            std::vector<Attribute> _outputAttribute;
        };

        Shader() noexcept;
        ~Shader();

        void init(const stream::Stream* stream, const ResourceHeader* header) override;
        bool load() override;

    private:

        const ShaderHeader* getShaderHeader() const;

        ReflectionInfo m_reflectionInfo;

        u32     m_size;
        void*   m_source;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
