#pragma once

#include "Common.h"
#include "Formats.h"
#include "crc32c/crc32c.h"

#define USE_STRING_ID_SHADER 1

namespace v3d
{
namespace resource
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief ShaderSourceBuildFlag enum.
    * Flags uses inside childs of ResourceLoader
    */
    enum ShaderSourceBuildFlag
    {
        ShaderSource_DontUseReflection = 1 << 0,

        ShaderSource_OptimizationSize = 1 << 1,
        ShaderSource_OptimizationPerformance = 1 << 2,
        ShaderSource_OptimizationFull = 1 << 3,

        ShaderSource_Patched = 1 << 4,
        ShaderSource_UseDXCompiler = 1 << 5, //For D3D shader should be signed or feature ExperimentalShaderModels is enabled
    };

    using ShaderSourceBuildFlags = u32;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource

namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    constexpr u32 k_maxDescriptorSetIndex = 4;
    constexpr u32 k_maxDescriptorBindingIndex = 8;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Shader;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief ShaderType enum
    */
    enum ShaderType : s16
    {
        ShaderType_Undefined = -1,

        Vertex = 0,
        Fragment = 1,

        Compute = 2,

        ShaderType_Count,
    };


    /**
    * @brief DataType enum
    */
    enum DataType : s32
    {
        DataType_None = -1,

        DataType_Int,
        DataType_Float,
        DataType_Double,

        DataType_Vector2,
        DataType_Vector3,
        DataType_Vector4,

        DataType_Matrix3,
        DataType_Matrix4,

        DataType_Struct,

        DataType_Count
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief ShaderProgramDescription struct
    */
    struct ShaderProgramDescription
    {
        ShaderProgramDescription() noexcept
            : _hash(0)
        {
            _shaders.fill(nullptr);
        }

        u32 _hash;
        std::array<const Shader*, ShaderType::ShaderType_Count> _shaders;
    };

    /**
    * @brief ShaderParam struct
    */
    struct ShaderParam
    {
        ShaderParam(const std::string& name) noexcept
            : _name(name)
            , _id(crc32c::Crc32c(name))
        {
            //TODO: add id manager and id generator to preventing collisions
        }

        std::string _name;
        u32         _id;
    };


    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief ShaderTypeString function
    * @param ShaderType type
    * @return shader string name
    */
    inline std::string ShaderTypeString(ShaderType type)
    {
        switch (type)
        {
        case ShaderType::ShaderType_Undefined:
            return "undefined";

        case ShaderType::Vertex:
            return "vertex";

        case ShaderType::Fragment:
            return "fragment";

        case ShaderType::Compute:
            return "compute";

        default:
            ASSERT(false, "not found");
        }

        return "unknown";
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
