#pragma once

#include "ResourceLoader.h"

namespace v3d
{
namespace renderer
{
    class Context;
} //namespace renderer

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

        ShaderSourceLoader(const renderer::Context* context, const std::vector<std::pair<std::string, std::string>>& defines) noexcept;
        ~ShaderSourceLoader();

        Shader* load(const std::string& name, const std::string& alias = "") override;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
