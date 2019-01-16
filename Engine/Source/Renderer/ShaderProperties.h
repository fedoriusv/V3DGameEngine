#pragma once

#include "Common.h"
#include "Formats.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * ShaderProgramDescription
    */
    struct ShaderProgramDescription
    {
        struct Attribute
        {
            u32     _location;
            u32     _offset;
            Format  _format;
        };

        std::map<std::string, Attribute> _inputAttachment;
        std::map<std::string, Attribute> _outputAttachment;
    };


    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
