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

    enum ShaderBinaryBuildFlag
    {
        ShaderBinary_DontUseReflaction = 1 << 0,
    };

    typedef u32 ShaderBinaryBuildFlags;

    /**
    * ShaderBinaryFileLoader class. Loader from file
    */
    class ShaderBinaryFileLoader : public ResourceLoader<renderer::Shader*>
    {
    public:

        ShaderBinaryFileLoader(const renderer::Context* context, const std::vector<std::pair<std::string, std::string>>& defines, ShaderBinaryBuildFlags flags = 0) noexcept;
        ~ShaderBinaryFileLoader();

        renderer::Shader* load(const std::string& name, const std::string& alias = "") override;

    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
