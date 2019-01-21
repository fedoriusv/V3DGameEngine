#pragma once

#include "Common.h"
#include "Formats.h"

namespace v3d
{
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
        struct Uniform
        {
            u32     _binding;
        };

        struct Texture
        {
            u32     _binding;
        };

        struct Attribute
        {
            u32     _location;
            u32     _offset;
            Format  _format;
        };


        std::map<std::string, Attribute> _inputAttachment;
        std::map<std::string, Attribute> _outputAttachment;
        std::map<std::string, Uniform>   _uniforms;
        std::map<std::string, Texture>   _textures;
    };


    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
