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

    /**
    * @brief k_invalidBinding
    */
    constexpr u32 k_invalidBinding = -1;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief ShaderProgram class. Game side
    */
    class ShaderProgram : public Object
    {
    public:

        explicit ShaderProgram(Device* device, const VertexShader* vertex) noexcept;
        explicit ShaderProgram(Device* device, const VertexShader* vertex, const FragmentShader* fragment) noexcept;
        explicit ShaderProgram(Device* device, const ComputeShader* compute) noexcept;

        const Shader* getShader(ShaderType type) const;

        u32 getResourceSlot(u32 set, u32 binding) const;
        u32 getResourceBinding(const std::string& name) const;

    private:

        Device* m_device;

        std::array<const Shader*, toEnumType(ShaderType::Count)> m_shaders;
        u32 m_shaderMask;

        u32 m_resourceSlots[k_maxDescriptorSetCount][k_maxDescriptorSetCount * k_maxDescriptorSlotsCount];
        std::map<std::string, u32> m_resourceBindings;
    };

    inline const Shader* ShaderProgram::getShader(ShaderType type) const
    {
        ASSERT(type <= ShaderType::Last, "range out");
        return m_shaders[toEnumType(type)];
    }

    inline u32 ShaderProgram::getResourceSlot(u32 set, u32 binding) const
    {
        ASSERT(set < k_maxDescriptorSetCount && binding < k_maxDescriptorSetCount * k_maxDescriptorSlotsCount, "out of range");
        ASSERT(m_resourceSlots[set][binding] != k_invalidBinding, "wrong binding");
        return m_resourceSlots[set][binding];
    }

    inline u32 ShaderProgram::getResourceBinding(const std::string& name) const
    {
        auto found = m_resourceBindings.find(name);
        if (found != m_resourceBindings.cend())
        {
            return found->second;
        }

        return k_invalidBinding;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
