#include "ShaderProgram.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace renderer
{

ShaderProgram::ShaderProgram(Device* device, const VertexShader* vertex, const FragmentShader* fragment) noexcept
    : m_device(device)
{
    m_shaders.fill(nullptr);

    ASSERT(vertex && fragment, "must be valid");
    m_shaders[toEnumType(ShaderType::Vertex)] = vertex;
    m_shaders[toEnumType(ShaderType::Fragment)] = fragment;

    m_shaderMask = 0b011;
}

ShaderProgram::ShaderProgram(Device* device, const ComputeShader* compute) noexcept
    : m_device(device)
{
    m_shaders.fill(nullptr);

    ASSERT(compute, "must be valid");
    m_shaders[toEnumType(ShaderType::Compute)] = compute;

    m_shaderMask = 0b100;
}

} //renderer
} //v3d
