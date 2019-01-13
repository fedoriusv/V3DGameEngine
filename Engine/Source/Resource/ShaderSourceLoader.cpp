#include "ShaderSourceLoader.h"

#include "ShaderSpirVDecoder.h"
#include "Shader.h"

#include "Stream/FileLoader.h"
#include "Renderer/Context.h"

namespace v3d
{
namespace resource
{

ShaderSourceLoader::ShaderSourceLoader(const renderer::Context* context, const std::vector<std::pair<std::string, std::string>>& defines) noexcept
{
    ASSERT(context, "context is nullptr");
    if (context->getRenderType() == renderer::Context::RenderType::VulkanRender)
    {
#ifdef USE_SPIRV
        {
            ShaderHeader header = {};
            header._contentType = ShaderHeader::ShaderResource::ShaderResource_Source;
            header._shaderLang = ShaderHeader::ShaderLang::ShaderLang_GLSL;
            header._apiVersion = 1.0f;
            header._optLevel = 0;
            header._defines = defines;

            ResourceLoader::registerDecoder(new ShaderSpirVDecoder({ "vert", "frag" }, header, true));
        }

        {
            ShaderHeader header;
            header._contentType = ShaderHeader::ShaderResource::ShaderResource_Source;
            header._shaderLang = ShaderHeader::ShaderLang::ShaderLang_HLSL;
            header._apiVersion = 1.0f;
            header._optLevel = 0;
            header._defines = defines;

            ResourceLoader::registerDecoder(new ShaderSpirVDecoder({"hlsl"}, header, true));
        }
#else //USE_SPIRV
        ASSERT(false, "not implemented");
#endif //USE_SPIRV
    }

    ResourceLoader::registerPath("../../../../engine/");
}

ShaderSourceLoader::~ShaderSourceLoader()
{
}

Shader * ShaderSourceLoader::load(const std::string & name, const std::string & alias)
{
    for (std::string& path : m_pathes)
    {
        const std::string fullPath = path + name;
        stream::Stream* file = stream::FileLoader::load(fullPath);
        if (!file)
        {
            continue;
        }

        std::string fileExtension = stream::FileLoader::getFileExtension(name);
        ResourceDecoder* decoder = ResourceLoader::findDecoder(fileExtension);
        if (decoder)
        {
           Resource* resource = decoder->decode(file, name);
           file->close();

           delete file;

           if (!resource)
           {
               LOG_ERROR("ShaderSourceLoader: Streaming error read file [%s]", name.c_str());
               return nullptr;
           }

           if (!resource->load())
           {
               LOG_ERROR("ShaderSourceLoader: Streaming error read file [%s]", name.c_str());
               return nullptr;
           }

           LOG_DEBUG("ShaderSourceLoader::load Shader [%s] is loaded", name.c_str());
           return static_cast<Shader*>(resource);
        }
    }

    LOG_WARNING("ShaderSourceLoader::load: File [%s] not found", name.c_str());
    return nullptr;
}

} //namespace resource
} //namespace v3d
