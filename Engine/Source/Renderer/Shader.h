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
    * @brief ShaderHeader meta info about Shader
    */
    struct ShaderHeader : resource::ResourceHeader
    {
        /**
        * @brief enum class ShaderContent
        */
        enum class ShaderContent : u16
        {
            Source,
            Bytecode,
        };

        /**
        * @brief enum class ShaderModel
        */
        enum class ShaderModel : u16
        {
            Default,

            GLSL_450,  //Default for GLSL
            HLSL_5_0,
            HLSL_5_1,  //Default for HLSL Legacy
            HLSL_6_1,  //Default for HLSL DXC
            HLSL_6_6,
            SpirV,
        };

        ShaderHeader() noexcept;
        ShaderHeader(const ShaderHeader& other) noexcept;
        explicit ShaderHeader(renderer::ShaderType type) noexcept;
        ~ShaderHeader() = default;

        u32 operator>>(stream::Stream* stream);
        u32 operator<<(const stream::Stream* stream);

        renderer::ShaderType _shaderType;
        ShaderContent        _contentType;
        ShaderModel          _shaderModel;
        u16                  _optLevel;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Shader class.
    * Resource, presents on Render and Game thread
    */
    class Shader : public resource::Resource
    {
    public:

        using DefineList = std::vector<std::pair<std::string, std::string>>;

        /**
        * @brief struct Attribute/Parameters
        */
        struct Attribute
        {
            Attribute() noexcept;

            u32              _location;
            renderer::Format _format;
            std::string      _name;

            u32 operator>>(stream::Stream* stream) const;
            u32 operator<<(const stream::Stream* stream);
        };

        /**
        * @brief struct UniformBuffer/ConstantBuffer
        */
        struct UniformBuffer
        {
            struct Uniform
            {
                Uniform() noexcept;

                u32                _bufferID;
                u32                _array;
                renderer::DataType _type;
                u32                _size;
                u32                _offset;
                std::string        _name;

                u32 operator>>(stream::Stream* stream) const;
                u32 operator<<(const stream::Stream* stream);
            };

            UniformBuffer() noexcept;

            u32                  _id;
            u32                  _set;
            u32                  _binding;
            u32                  _array;
            u32                  _size;
            std::string          _name;

            std::vector<Uniform> _uniforms;

            u32 operator>>(stream::Stream* stream) const;
            u32 operator<<(const stream::Stream* stream);
        };

        /**
        * @brief struct Image/Texture
        */
        struct Image
        {
            Image() noexcept;

            u32                     _set;
            u32                     _binding;
            renderer::TextureTarget _target;
            u32                     _array;
            bool                    _depth;
            bool                    _ms;
            std::string             _name;

            u32 operator>>(stream::Stream* stream) const;
            u32 operator<<(const stream::Stream* stream);
        };

        /**
        * @brief struct Sampler
        */
        struct Sampler
        {
            Sampler() noexcept;

            u32                     _set;
            u32                     _binding;
            std::string             _name;

            u32 operator>>(stream::Stream* stream) const;
            u32 operator<<(const stream::Stream* stream);
        };

        /**
        * @brief struct StorageImage/UAV
        */
        struct StorageImage
        {
            StorageImage() noexcept;

            u32                     _set;
            u32                     _binding;
            renderer::TextureTarget _target;
            renderer::Format        _format;
            u32                     _array;
            bool                    _readonly;
            std::string             _name;

            u32 operator>>(stream::Stream* stream) const;
            u32 operator<<(const stream::Stream* stream);
        };

        /**
        * @brief struct StorageBuffer/UAV
        */
        struct StorageBuffer
        {
            StorageBuffer() noexcept;

            u32                     _set;
            u32                     _binding;
            renderer::Format        _format;
            u32                     _array;
            bool                    _readonly;
            std::string             _name;

            u32 operator>>(stream::Stream* stream) const;
            u32 operator<<(const stream::Stream* stream);
        };

        /**
        * @brief struct PushConstant
        */
        struct PushConstant
        {
            PushConstant() noexcept;

            u32         _offset;
            u32         _size;
            std::string _name;

            u32 operator>>(stream::Stream* stream) const;
            u32 operator<<(const stream::Stream* stream);
        };

        /**
        * @brief struct ReflectionInfo
        */
        struct ReflectionInfo
        {
            std::vector<Attribute>      _inputAttribute;
            std::vector<Attribute>      _outputAttribute;
            std::vector<UniformBuffer>  _uniformBuffers;
            std::vector<Image>          _sampledImages;
            std::vector<Image>          _images;
            std::vector<Sampler>        _samplers;
            std::vector<StorageImage>   _storageImages;
            std::vector<StorageBuffer>  _storageBuffers;
            std::vector<PushConstant>   _pushConstant;
        };

        Shader() noexcept;
        explicit Shader(ShaderHeader* header) noexcept;
        ~Shader();

        renderer::ShaderType getShaderType() const;
        const std::string& getEntrypoint() const;

        const ReflectionInfo& getReflectionInfo() const;

    private:

        Shader(const Shader&) = delete;

        bool load(const stream::Stream* stream, u32 offset = 0) override;
        bool save(stream::Stream* stream, u32 offset = 0) const override;

        ShaderHeader* m_header;

        u32 m_hash;
        u32 m_size;
        void* m_source;
        std::string m_entrypoint;

        ReflectionInfo  m_reflectionInfo;

        friend renderer::Pipeline;
        friend renderer::ShaderProgram;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    inline const Shader::ReflectionInfo& Shader::getReflectionInfo() const
    {
        return m_reflectionInfo;
    }

    inline const std::string& Shader::getEntrypoint() const
    {
        return m_entrypoint;
    }

    inline renderer::ShaderType Shader::getShaderType() const
    {
        ASSERT(m_header, "nullptr");
        return m_header->_shaderType;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
