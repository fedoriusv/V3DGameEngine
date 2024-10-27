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

        const Shader* getShader(ShaderType type) const;

    private:

        Device* m_device;

        std::array<const Shader*, toEnumType(ShaderType::Count)> m_shaders;
        u32 m_shaderMask;
    };

    inline const Shader* ShaderProgram::getShader(ShaderType type) const
    {
        ASSERT(type <= ShaderType::Last, "range out");
        return m_shaders[toEnumType(type)];
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
