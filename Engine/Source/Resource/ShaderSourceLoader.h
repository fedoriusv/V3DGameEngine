#pragma once

#include "ResourceLoader.h"

namespace v3d
{
namespace renderer
{
    class Context;
}
namespace resource
{
    class Shader;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * ShaderHeader meta info about Shader
    */
    class ShaderSourceLoader : public ResourceLoader<Shader*>
    {
    public:

        ShaderSourceLoader(renderer::Context* context) noexcept;
        ~ShaderSourceLoader();

        Shader* load(const std::string& name, const std::string& alias = "") override;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
