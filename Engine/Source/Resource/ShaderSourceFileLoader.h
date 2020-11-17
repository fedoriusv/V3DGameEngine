#pragma once

#include "ResourceLoader.h"
#include "Renderer/ShaderProperties.h"

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
    * @brief ShaderSourceBuildFlag enum.
    * Flags uses inside ShaderSourceFileLoader
    */
    enum ShaderSourceBuildFlag
    {
        ShaderSource_DontUseReflection = 1 << 0,
        ShaderSource_OptimisationSize = 1 << 1,
        ShaderSource_OptimisationPerformance = 1 << 2,
        ShaderSource_Patched = 1 << 3,
    };

    typedef u32 ShaderSourceBuildFlags;

    /**
    * @brief ShaderSourceFileLoader class. Resource loader.
    * Loads shaders for source file
    */
    class ShaderSourceFileLoader : public ResourceLoader<renderer::Shader*>
    {
    public:

        /**
        * @brief ShaderSourceFileLoader constructor.
        * @param const renderer::Context* context [required]
        * @param const std::vector<std::pair<std::string, std::string>>& defines [optional]
        * @param ShaderSourceBuildFlags flags [optional] @see ShaderSourceBuildFlags
        */
        ShaderSourceFileLoader(const renderer::Context* context, const std::vector<std::pair<std::string, std::string>>& defines = {}, ShaderSourceBuildFlags flags = 0) noexcept;

        /**
        * @brief ShaderSourceFileLoader constructor.
        * Support HLSL format only
        * @param const renderer::Context* context [required]
        * @param renderer::ShaderType type [required]
        * @param const std::string& entryPoint [optional]
        * @param const std::vector<std::pair<std::string, std::string>>& defines [optional]
        * @param ShaderSourceBuildFlags flags [optional] @see ShaderSourceBuildFlags
        */
        ShaderSourceFileLoader(const renderer::Context* context, renderer::ShaderType type, const std::string& entryPoint = "main", const std::vector<std::pair<std::string, std::string>>& defines = {}, ShaderSourceBuildFlags flags = 0) noexcept;

        ~ShaderSourceFileLoader();

        renderer::Shader* load(const std::string& name, const std::string& alias = "") override;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
