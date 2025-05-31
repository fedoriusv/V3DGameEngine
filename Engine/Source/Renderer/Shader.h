#pragma once

#include "Render.h"
#include "Formats.h"
#include "Resource/Resource.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief ShaderType enum
    */
    enum class ShaderType : u32
    {
        Vertex = 0,
        Fragment = 1,
        Compute = 2,

        First = Vertex,
        Last = Compute,
        Count,
    };

    /**
    * @brief ShaderTypeString function
    * @param ShaderType type
    * @return shader string name
    */
    std::string ShaderTypeString(ShaderType type);

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief enum class ShaderContent
    */
    enum class ShaderContent : u32
    {
        Source,
        Bytecode,
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief enum class ShaderModel
    */
    enum class ShaderModel : u32
    {
        //GL = GLSL_450, HLSL = HLSL_6_1
        Default,

        //GLSL
        GLSL_450,

        //HLSL Legacy
        HLSL_5_1,

        //HLSL DXC
        HLSL_6_0,
        HLSL_6_1, //Default
        HLSL_6_2,
        HLSL_6_3,
        HLSL_6_4,
        HLSL_6_5,
        HLSL_6_6,

        GLSL = GLSL_450,
        HLSL = HLSL_6_1,
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief DataType enum
    */
    enum class DataType : s32
    {
        None = -1,

        Int16,
        UInt16,

        Int32,
        UInt32,

        Int64,
        UInt64,

        Float16,
        Float32,
        Float64,

        Vector2,
        Vector3,
        Vector4,

        Matrix3,
        Matrix4,

        Struct,

        Count
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Shader base class.
    * Resource, presents on Render and Game thread
    */
    class Shader : public resource::Resource
    {
    public:

        using DefineList = std::vector<std::pair<std::string, std::string>>;

        /**
        * @brief ShaderHeader struct.
        */
        struct ShaderHeader : resource::ResourceHeader
        {
            ShaderHeader() noexcept
                : resource::ResourceHeader(resource::ResourceType::Shader)
            {
            }
        };

        /**
        * @brief ShaderData struct.
        */
        struct ShaderData
        {
            u32     _size;
            void*   _data;
        };

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
        * @brief struct Resources
        */
        struct Resources
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

        ShaderType getType() const;
        const Resources& getMappingResources() const;

    public:

        explicit Shader(const ShaderHeader& header) noexcept;
        virtual ~Shader();

        const std::string& getEntryPoint() const;
        const void* getBytecode() const;
        u32 getBytecodeSize() const;

    protected:

        explicit Shader(ShaderType type) noexcept;

        bool load(const stream::Stream* stream, u32 offset = 0) override;
        bool save(stream::Stream* stream, u32 offset = 0) const override;

        ShaderHeader    m_header;
        ShaderType      m_type;
        ShaderModel     m_shaderModel;
        std::string     m_entryPoint;
        ShaderData      m_data;

        Resources       m_mappedResources;
    };

    inline ShaderType Shader::getType() const
    {
        return m_type;
    }

    inline const std::string& Shader::getEntryPoint() const
    {
        return m_entryPoint;
    }

    inline const void* Shader::getBytecode() const
    {
        return m_data._data;
    }
    
    inline u32 Shader::getBytecodeSize() const
    {
        return m_data._size;
    }

    inline const Shader::Resources& Shader::getMappingResources() const
    {
        return m_mappedResources;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VertexShader class.
    */
    class VertexShader : public Shader
    {
    public: 

        VertexShader() noexcept;
    };

    /**
    * @brief FragmentShader class.
    */
    class FragmentShader : public Shader
    {
    public:

        FragmentShader() noexcept;
    };

    /**
    * @brief ComputeShader class.
    */
    class ComputeShader : public Shader
    {
    public:

        ComputeShader() noexcept;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    template<class TShader>
    inline ShaderType getShaderTypeByClass()
    {
        static_assert(std::is_base_of<renderer::Shader, TShader>(), "wrong type");
        if constexpr (std::is_same<TShader, VertexShader>::value)
        {
            return ShaderType::Vertex;
        }
        else if constexpr (std::is_same<TShader, FragmentShader>::value)
        {
            return ShaderType::Fragment;
        }
        else if constexpr (std::is_same<TShader, ComputeShader>::value)
        {
            return ShaderType::Compute;
        }

        return ShaderType::First;
    }

} //namespace renderer
} //namespace v3d
