#pragma once

#include "Resource/Resource.h"
#include "Formats.h"
#include "TextureProperties.h"
#include "ShaderProperties.h"

namespace v3d
{
namespace renderer
{
    class ShaderProgram;
    class Pipeline;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * ShaderHeader meta info about Shader
    */
    struct ShaderHeader : resource::ResourceHeader
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
            ShaderLang_CG,
            ShaderLang_SpirV
        };

        ShaderHeader() noexcept;
        explicit ShaderHeader(renderer::ShaderType type) noexcept;

        renderer::ShaderType _type;
        ShaderResource _contentType;
        ShaderLang     _shaderLang;
        u32            _apiVersion;
        u32            _optLevel;
        std::string    _entyPoint;

        std::vector<std::pair<std::string, std::string>> _defines;
        std::vector<std::string>                         _include;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Shader class. 
    * Resource, present on Render and Game thread
    */
    class Shader : public resource::Resource
    {
    public:

        struct Attribute
        {
            Attribute();

            u32              _location;
            renderer::Format _format;
#if USE_STRING_ID_SHADER
            std::string      _name;
#endif
            void operator >> (stream::Stream * stream) const;
            void operator << (const stream::Stream * stream);
        };

        struct UniformBuffer
        {
            struct Uniform
            {
                Uniform();

                u32                _bufferId;
                u32                _array;
                renderer::DataType _type;
                u32                _size;
#if USE_STRING_ID_SHADER
                std::string        _name;
#endif
                void operator >> (stream::Stream * stream) const;
                void operator << (const stream::Stream * stream);
            };

            UniformBuffer();

            u32                  _id;
            u32                  _set;
            u32                  _binding;
            u32                  _array;
            u32                  _size;
#if USE_STRING_ID_SHADER
            std::string          _name;
#endif
            std::vector<Uniform> _uniforms;

            void operator >> (stream::Stream * stream) const;
            void operator << (const stream::Stream * stream);
        };

        struct SampledImage
        {
            SampledImage();

            u32                     _set;
            u32                     _binding;
            renderer::TextureTarget _target;
            u32                     _array;
            bool                    _depth;
            bool                    _ms;
#if USE_STRING_ID_SHADER
            std::string             _name;
#endif
            void operator >> (stream::Stream * stream) const;
            void operator << (const stream::Stream * stream);
        };

        struct PushConstant
        {
            PushConstant();

            u32         _offset;
            u32         _size;
#if USE_STRING_ID_SHADER
            std::string _name;
#endif
            void operator >> (stream::Stream * stream) const;
            void operator << (const stream::Stream * stream);
        };

        struct ReflectionInfo
        {
            std::vector<Attribute>      _inputAttribute;
            std::vector<Attribute>      _outputAttribute;
            std::vector<UniformBuffer>  _uniformBuffers;
            std::vector<SampledImage>   _sampledImages;
            std::vector<PushConstant>   _pushConstant;
        };

        explicit Shader(const ShaderHeader* header) noexcept;
        ~Shader();

        void init(stream::Stream* stream) override;
        bool load() override;

        const ShaderHeader& getShaderHeader() const;
        const ReflectionInfo& getReflectionInfo() const;

    private:

        ReflectionInfo  m_reflectionInfo;

        u32             m_hash;
        u32             m_size;
        void*           m_source;

        friend renderer::Pipeline;
        friend renderer::ShaderProgram;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
