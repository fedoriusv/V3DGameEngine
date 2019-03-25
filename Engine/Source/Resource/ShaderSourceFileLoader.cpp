#include "ShaderSourceFileLoader.h"

#include "Renderer/Context.h"
#include "Stream/FileLoader.h"
#include "Resource/ResourceLoaderManager.h"
#include "Renderer/Shader.h"

#ifdef USE_SPIRV
#include "ShaderSpirVDecoder.h"
#endif //USE_SPIRV

namespace v3d
{
namespace resource
{

ShaderSourceFileLoader::ShaderSourceFileLoader(const renderer::Context* context, const std::vector<std::pair<std::string, std::string>>& defines) noexcept
{
    ASSERT(context, "context is nullptr");
    if (context->getRenderType() == renderer::Context::RenderType::VulkanRender)
    {
#ifdef USE_SPIRV
        {
            renderer::ShaderHeader header;
            header._contentType = renderer::ShaderHeader::ShaderResource::ShaderResource_Source;
            header._shaderLang = renderer::ShaderHeader::ShaderLang::ShaderLang_GLSL;
            header._optLevel = 0;
            header._defines = defines;

            ResourceLoader::registerDecoder(new ShaderSpirVDecoder({ "vert", "frag" }, header, true));
        }

        {
            renderer::ShaderHeader header;
            header._contentType = renderer::ShaderHeader::ShaderResource::ShaderResource_Source;
            header._shaderLang = renderer::ShaderHeader::ShaderLang::ShaderLang_HLSL;
            header._optLevel = 0;
            header._defines = defines;

            ResourceLoader::registerDecoder(new ShaderSpirVDecoder({"hlsl"}, header, true));
        }
#else //USE_SPIRV
        ASSERT(false, "not implemented");
#endif //USE_SPIRV
    }

    ResourceLoader::registerPath("../../../../");
    ResourceLoader::registerPath("../../../../../");
    ResourceLoader::registerPath("../../../../engine/");
    ResourceLoader::registerPathes(ResourceLoaderManager::getInstance()->getPathes());
}

ShaderSourceFileLoader::~ShaderSourceFileLoader()
{

}

renderer::Shader * ShaderSourceFileLoader::load(const std::string & name, const std::string & alias)
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
               LOG_ERROR("ShaderSourceFileLoader: Streaming error read file [%s]", name.c_str());
               return nullptr;
           }

           if (!resource->load())
           {
               LOG_ERROR("ShaderSourceFileLoader: Streaming error read file [%s]", name.c_str());
               return nullptr;
           }

           LOG_DEBUG("ShaderSourceFileLoader::load Shader [%s] is loaded", name.c_str());
           return static_cast<renderer::Shader*>(resource);
        }
    }

    LOG_WARNING("ShaderSourceFileLoader::load: File [%s] not found", name.c_str());
    return nullptr;
}

} //namespace resource
} //namespace v3d
