#pragma once

#include "Common.h"
#include "Renderer/Shader.h"

#if defined(PLATFORM_WINDOWS) || defined(PLATFORM_XBOX)

struct IDxcBlob;

namespace v3d
{
namespace resource
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * ShaderReflectionDXC. Shader Reflection for DXIL
    */
    class ShaderReflectionDXC final
    {
    public:

        ShaderReflectionDXC() noexcept;
        ~ShaderReflectionDXC();

        bool reflect(const IDxcBlob* shader, stream::Stream* stream) const;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
#endif //PLATFORM