#pragma once

#include "Common.h"
#include "Formats.h"

namespace v3d
{
namespace resource
{
    class Shader;
}
namespace renderer
{

    /////////////////////////////////////////////////////////////////////////////////////////////////////

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
        }
        ShaderProgramDescription(const ShaderProgramDescription&) = default;
        ShaderProgramDescription& operator=(const ShaderProgramDescription&) = default;

        ShaderProgramDescription(ShaderProgramDescription&& desc) noexcept
        {
            _hash = desc._hash;
            _shaders = std::move(desc._shaders);
        }

        ShaderProgramDescription& operator=(ShaderProgramDescription&& desc) noexcept
        {
            if (this == &desc)
            {
                return *this;
            }

            _hash = desc._hash;
            _shaders = std::move(desc._shaders);
        }

        u32 _hash;
        std::vector<resource::Shader*> _shaders;
    };


    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
