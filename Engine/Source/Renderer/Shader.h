#pragma once

#include "Resource/Resource.h"

#include "Formats.h"
#include "TextureProperties.h"
#include "ShaderProperties.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class ShaderProgram;
    class Pipeline;

    /**
    * @brief ShaderHeader meta info about Shader
    */
    struct ShaderHeader : resource::ResourceHeader
    {
        /**
        * @brief enum class ShaderResource
        */
        enum class ShaderResource : u16
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

            GLSL_450,
            HLSL_5_0,
            HLSL_5_1,
            HLSL_6_0,
            HLSL_6_1,
            SpirV,
        };

        ShaderHeader() noexcept;
        explicit ShaderHeader(renderer::ShaderType type) noexcept;

        renderer::ShaderType _type;
        ShaderResource       _contentType;
        ShaderModel          _shaderModel;
        u32                  _optLevel;
        std::string          _entryPoint;

        std::vector<std::pair<std::string, std::string>> _defines;
        std::vector<std::string>                         _include;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Shader class.
    * Resource, present on Render and Game thread
    */
    class Shader : public resource::Resource
    {
    public:

        /**
        * @brief struct Attribute
        */
        struct Attribute
        {
            Attribute() noexcept;

            u32              _location;
            renderer::Format _format;
#if USE_STRING_ID_SHADER
            std::string      _name;
#endif
            void operator>>(stream::Stream* stream) const;
            void operator<<(const stream::Stream* stream);
        };

        /**
        * @brief struct UniformBuffer
        */
        struct UniformBuffer
        {
            struct Uniform
            {
                Uniform() noexcept;

                u32                _bufferId;
                u32                _array;
                renderer::DataType _type;
                u32                _size;
                u32                _offset;
#if USE_STRING_ID_SHADER
                std::string        _name;
#endif
                void operator>>(stream::Stream* stream) const;
                void operator<<(const stream::Stream* stream);
            };

            UniformBuffer() noexcept;

            u32                  _id;
            u32                  _set;
            u32                  _binding;
            u32                  _array;
            u32                  _size;
#if USE_STRING_ID_SHADER
            std::string          _name;
#endif
            std::vector<Uniform> _uniforms;

            void operator>>(stream::Stream* stream) const;
            void operator<<(const stream::Stream* stream);
        };

        /**
        * @brief struct Image
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
#if USE_STRING_ID_SHADER
            std::string             _name;
#endif
            void operator>>(stream::Stream* stream) const;
            void operator<<(const stream::Stream* stream);
        };

        /**
        * @brief struct Sampler
        */
        struct Sampler
        {
            Sampler() noexcept;

            u32                     _set;
            u32                     _binding;
#if USE_STRING_ID_SHADER
            std::string             _name;
#endif
            void operator>>(stream::Stream* stream) const;
            void operator<<(const stream::Stream* stream);
        };

        /**
        * @brief struct StorageImage
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
#if USE_STRING_ID_SHADER
            std::string             _name;
#endif

            void operator>>(stream::Stream* stream) const;
            void operator<<(const stream::Stream* stream);
        };

        /**
        * @brief struct StorageBuffer
        */
        struct StorageBuffer
        {
            StorageBuffer() noexcept;

            u32                     _set;
            u32                     _binding;
#if USE_STRING_ID_SHADER
            std::string             _name;
#endif

            void operator>>(stream::Stream* stream) const;
            void operator<<(const stream::Stream* stream);
        };

        /**
        * @brief struct PushConstant
        */
        struct PushConstant
        {
            PushConstant() noexcept;

            u32         _offset;
            u32         _size;
#if USE_STRING_ID_SHADER
            std::string _name;
#endif
            void operator>>(stream::Stream* stream) const;
            void operator<<(const stream::Stream* stream);
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
