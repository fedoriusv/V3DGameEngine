#pragma once

#include "Common.h"
#include "Renderer/Shader.h"

#ifdef USE_SPIRV
namespace v3d
{
namespace resource
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * ShaderReflectionSpirV. Shader Reflection for SpirV
    */
    class ShaderReflectionSpirV final
    {
    public:

        ShaderReflectionSpirV(renderer::ShaderHeader::ShaderModel model) noexcept;
        ~ShaderReflectionSpirV();

        bool reflect(const std::vector<u32>& spirv, stream::Stream* stream);

    private:

        const renderer::ShaderHeader::ShaderModel m_model;
        u32 m_version;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
#endif //USE_SPIRV