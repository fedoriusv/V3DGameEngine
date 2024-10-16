#pragma once

#include "Object.h"
#include "Render.h"
#include "Shader.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Device;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief ShaderProgram class. Game side
    */
    class ShaderProgram : public Object
    {
    public:

        explicit ShaderProgram(Device* device, const VertexShader* vertex, const FragmentShader* fragment) noexcept;
        explicit ShaderProgram(Device* device, const ComputeShader* compute) noexcept;

    private:

        Device* m_device;

        std::array<const Shader*, toEnumType(ShaderType::Count)> m_shaders;
        u32 m_shaderMask;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
