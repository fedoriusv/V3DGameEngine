#include "ShaderProgram.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace renderer
{

ShaderProgram::ShaderProgram(Device* device, const VertexShader* vertex, const FragmentShader* fragment) noexcept
    : m_device(device)
{
    ASSERT(vertex && fragment, "must be valid");
    m_shaders[toEnumType(ShaderType::Vertex)] = vertex;
    m_shaders[toEnumType(ShaderType::Fragment)] = fragment;


}

ShaderProgram::ShaderProgram(Device* device, const ComputeShader* compute) noexcept
    : m_device(device)
{
}

} //renderer
} //v3d
