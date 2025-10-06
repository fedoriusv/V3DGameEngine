#pragma once

#include "Common.h"

namespace v3d
{
namespace scene
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    enum class RenderPipelinePass : u32
    {
        Opaque,
        SkinnedOpaque,
        MaskedOpaque,

        Transparency,
        SkinnedTransparency,

        Billboard,
        VFX,

        DirectionLight,
        PunctualLights,
        Shadowmap,

        Selected,
        Indicator,
        Debug,

        Custom,

        Count
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define SHADER_PARAMETER(parameter) \
    u32 parameter = -1

#ifdef DEBUG
#define BIND_SHADER_PARAMETER_VALIDATE(parameter, name, path) \
     if (parameter == renderer::k_invalidBinding) LOG_WARNING("bindDescriptorSet: Parameter of descriptor set [%s] has unbound resource. %s", name, path);
#else
#define BIND_SHADER_PARAMETER_VALIDATE(parameter)
#endif // DEBUG


#define BIND_SHADER_PARAMETER(pipeline, material, parameter) \
    material.parameter = pipeline->getShaderProgram()->getResourceBinding(#parameter); \
    BIND_SHADER_PARAMETER_VALIDATE(material.parameter, #parameter, __FILE_PATH__)

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace scene
} // namespace v3d