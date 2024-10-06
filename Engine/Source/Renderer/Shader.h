#pragma once

#include "Render.h"
#include "Formats.h"
#include "Utils/Resource.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief ShaderType enum
    */
    enum class ShaderType : s32
    {
        Undefined = -1,

        Vertex = 0,
        Fragment = 1,
        Compute = 2,

        Count,
    };

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

    struct ShaderData
    {

    };

    /**
    * @brief Shader class.
    * Resource, presents on Render and Game thread
    */
    class Shader : public utils::Resource
    {
    public:

        explicit Shader(ShaderType type) noexcept
            : m_type(type)
        {
        }

        virtual ~Shader() = default;

    protected:

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
