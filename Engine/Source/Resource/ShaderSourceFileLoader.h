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
    * ShaderSourceFileLoader class. Loader from file
    */
    class ShaderSourceFileLoader : public ResourceLoader<Shader*>
    {
    public:

        ShaderSourceFileLoader(const renderer::Context* context, const std::vector<std::pair<std::string, std::string>>& defines) noexcept;
        ~ShaderSourceFileLoader();

        Shader* load(const std::string& name, const std::string& alias = "") override;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
