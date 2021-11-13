#pragma once

#include "Common.h"
#include "Renderer/Shader.h"

#ifdef PLATFORM_WINDOWS

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
#endif //PLATFORM_WINDOWS