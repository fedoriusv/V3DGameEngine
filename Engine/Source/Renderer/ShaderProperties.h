#pragma once

#include "Common.h"
#include "Formats.h"

namespace v3d
{
namespace renderer
{
    class Shader;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief k_maxDescriptorBindingCount
    */
    constexpr u32 k_maxDescriptorSetCount = 4;

    /**
    * @brief k_maxDescriptorBindingCount
    */
    constexpr u32 k_maxDescriptorBindingCount = 8;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief ShaderType enum
    */
    enum class ShaderType : s16
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
        ShaderProgramDescription() noexcept;

        u32 _hash;
        std::array<const Shader*, toEnumType(ShaderType::Count)> _shaders;
    };

    /**
    * @brief ShaderParam struct
    */
    struct ShaderParam
    {
        ShaderParam(const std::string& name) noexcept;

        std::string _name;
        u32         _id;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief ShaderTypeString function
    * @param ShaderType type
    * @return shader string name
    */
    std::string ShaderTypeString(ShaderType type);

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief ShaderCompileFlag enum.
    */
    enum ShaderCompileFlag
    {
        ShaderSource_DontUseReflection = 1 << 0,

        ShaderSource_OptimizationSize = 1 << 1,
        ShaderSource_OptimizationPerformance = 1 << 2,
        ShaderSource_OptimizationFull = 1 << 3,

        ShaderSource_Patched = 1 << 4,
        ShaderSource_UseLegacyCompilerForHLSL = 1 << 5,
        ShaderSource_UseDXCompilerForSpirV = 1 << 6,
    };
    typedef u32 ShaderCompileFlags;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
