#pragma once

#include "Common.h"
#include "Formats.h"
#include "crc32c/crc32c.h"

#define USE_STRING_ID_SHADER 1

namespace v3d
{
namespace renderer
{
    class Shader;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    enum ShaderType : s16
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

        DataType_Struct,

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
            _shaders.fill(nullptr);
        }

        u32 _hash;
        std::array<const Shader*, ShaderType::ShaderType_Count> _shaders;
    };

    /**
    * ShaderParam
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

} //namespace renderer
} //namespace v3d
