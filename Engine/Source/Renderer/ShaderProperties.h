#pragma once

#include "Common.h"
#include "Formats.h"

namespace v3d
{
namespace renderer
{
    class Shader;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    enum ShaderType : s32
    {
        ShaderType_Undefined = -1,
        ShaderType_Vertex = 0,
        ShaderType_Fragment,

        ShaderType_Count,
    };


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

        DataType_Count
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * ShaderProgramDescription
    */
    struct ShaderProgramDescription
    {
        ShaderProgramDescription() noexcept
            : _hash(0)
        {
            memset(&_shaders, 0, sizeof(_shaders));
        }
        ShaderProgramDescription(const ShaderProgramDescription&) = default;
        ShaderProgramDescription& operator=(const ShaderProgramDescription&) = default;

        u32 _hash;
        std::array<const Shader*, ShaderType::ShaderType_Count> _shaders;
    };


    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
