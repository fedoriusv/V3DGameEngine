#include "ShaderProperties.h"

#include "crc32c/crc32c.h"


namespace v3d
{
namespace renderer
{

ShaderProgramDescription::ShaderProgramDescription() noexcept
    : _hash(0)
{
    _shaders.fill(nullptr);
}

ShaderParam::ShaderParam(const std::string& name) noexcept
    : _name(name)
    , _id(crc32c::Crc32c(name))
{
    //TODO: add id manager and id generator to preventing collisions
}

std::string ShaderTypeString(ShaderType type)
{
    switch (type)
    {
    case ShaderType::Undefined:
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

} //namespace renderer
} //namespace v3d