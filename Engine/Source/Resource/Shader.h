#pragma once

#include "Resource.h"
#include "Renderer/ImageFormats.h"

namespace v3d
{
namespace renderer
{
    class ShaderProgram;
    class Pipeline;
} //namespace renderer

namespace resource
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    enum ShaderType : s32
    {
        ShaderType_Undefined = -1,
        ShaderType_Vertex = 0,
        ShaderType_Fragment,

        ShaderType_Count,
    };

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

        ShaderHeader() noexcept;

        ShaderType     _type;
        ShaderResource _contentType;
        ShaderLang     _shaderLang;
        f32            _apiVersion;
        u32            _optLevel;
        std::string    _entyPoint;

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

        explicit Shader(const ShaderHeader* header) noexcept;
        ~Shader();

        void init(const stream::Stream* stream) override;
        bool load() override;

        const ShaderHeader* getShaderHeader() const;

    private:

        ReflectionInfo m_reflectionInfo;

        u32     m_size;
        void*   m_source;

        friend renderer::Pipeline;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
