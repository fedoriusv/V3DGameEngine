#pragma once

#include "ResourceLoader.h"

namespace v3d
{
namespace renderer
{
    class Context;
    class Shader;
} //namespace renderer

namespace resource
{

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * ShaderSourceFileLoader class. Loader from file
    */
    class ShaderSourceFileLoader : public ResourceLoader<renderer::Shader*>
    {
    public:

        ShaderSourceFileLoader(const renderer::Context* context, const std::vector<std::pair<std::string, std::string>>& defines) noexcept;
        ~ShaderSourceFileLoader();

        renderer::Shader* load(const std::string& name, const std::string& alias = "") override;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
