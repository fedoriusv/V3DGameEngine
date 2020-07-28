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

    enum ShaderSourceBuildFlag
    {
        ShaderSource_DontUseReflection = 1 << 0,
        ShaderSource_OptimisationSize = 1 << 1,
        ShaderSource_OptimisationPerformance = 1 << 2,
        ShaderSource_Patched = 1 << 3,
    };

    typedef u32 ShaderSourceBuildFlags;

    /**
    * ShaderSourceFileLoader class. Loader from file
    */
    class ShaderSourceFileLoader : public ResourceLoader<renderer::Shader*>
    {
    public:

        ShaderSourceFileLoader(const renderer::Context* context, const std::vector<std::pair<std::string, std::string>>& defines, ShaderSourceBuildFlags flags = 0) noexcept;
        ~ShaderSourceFileLoader();

        renderer::Shader* load(const std::string& name, const std::string& alias = "") override;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
