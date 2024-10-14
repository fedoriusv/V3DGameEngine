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

        Count,
    };

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

        Int,
        Float,
        Double,

        Vector2,
        Vector3,
        Vector4,

        Matrix3,
        Matrix4,

        Struct,

        Count
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    struct ShaderDesc
    {
        ShaderType _type;
    };

    struct ShaderData
    {

    };

    /**
    * @brief Shader class.
    * Resource, presents on Render and Game thread
    */
    class Shader : public resource::Resource
    {
    public:

        using DefineList = std::vector<std::pair<std::string, std::string>>;

        explicit Shader(ShaderType type) noexcept
            : m_type(type)
        {
        }

        virtual ~Shader() = default;

    protected:

        bool load(const stream::Stream* stream, u32 offset = 0) override;
        bool save(stream::Stream* stream, u32 offset = 0) const override;

        ShaderType m_type;
    };


    class GraphicShader : public Shader
    {
    };

    class ComputeShader : public Shader
    {
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
