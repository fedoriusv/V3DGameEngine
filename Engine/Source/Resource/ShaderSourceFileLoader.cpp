#include "ShaderSourceFileLoader.h"

#include "Renderer/Context.h"
#include "Stream/FileLoader.h"
#include "Resource/ResourceLoaderManager.h"
#include "Renderer/Shader.h"

#ifdef USE_SPIRV
#   include "ShaderSpirVDecoder.h"
#endif //USE_SPIRV

#if D3D_RENDER
#   include "ShaderHLSLDecoder.h"
#endif // D3Dre


namespace v3d
{
namespace resource
{

ShaderSourceFileLoader::ShaderSourceFileLoader(const renderer::Context* context, const std::vector<std::pair<std::string, std::string>>& defines, ShaderSourceBuildFlags flags) noexcept
{
    ASSERT(context, "context is nullptr");
    if (context->getRenderType() == renderer::Context::RenderType::VulkanRender)
    {
#ifdef USE_SPIRV
        {
#if USE_STRING_ID_SHADER
            ASSERT(!(flags & ShaderSourceBuildFlag::ShaderSource_OptimisationSize || flags & ShaderSourceBuildFlag::ShaderSource_OptimisationPerformance), "define can't work with the flag because optimisation removes names from spirv");
#endif
            renderer::ShaderHeader header;
            header._contentType = renderer::ShaderHeader::ShaderResource::ShaderResource_Source;
            header._shaderModel = renderer::ShaderHeader::ShaderModel::ShaderModel_GLSL_450;
            header._optLevel = (flags & ShaderSourceBuildFlag::ShaderSource_OptimisationPerformance) ? 2 : (flags & ShaderSourceBuildFlag::ShaderSource_OptimisationSize) ? 1 : 0;
            header._defines = defines;
            header._flags |= (flags & ShaderSourceBuildFlag::ShaderSource_Patched) ? ShaderSourceBuildFlag::ShaderSource_Patched : header._flags;

            ResourceLoader::registerDecoder(new ShaderSpirVDecoder( { "vert", "frag" }, header, !(flags & ShaderSourceBuildFlag::ShaderSource_DontUseReflection) ));
        }

        {
            renderer::ShaderHeader header;
            header._contentType = renderer::ShaderHeader::ShaderResource::ShaderResource_Source;
            header._shaderModel = renderer::ShaderHeader::ShaderModel::ShaderModel_HLSL_5_1;
            header._optLevel = (flags & ShaderSourceBuildFlag::ShaderSource_OptimisationPerformance) ? 2 : (flags & ShaderSourceBuildFlag::ShaderSource_OptimisationSize) ? 1 : 0;
            header._defines = defines;
            header._flags |= (flags & ShaderSourceBuildFlag::ShaderSource_Patched) ? ShaderSourceBuildFlag::ShaderSource_Patched : header._flags;

            ResourceLoader::registerDecoder(new ShaderSpirVDecoder( { "vs", "ps" }, header, !(flags & ShaderSourceBuildFlag::ShaderSource_DontUseReflection) ));
        }
#else //USE_SPIRV
        ASSERT(false, "not implemented");
#endif //USE_SPIRV
    }

#if D3D_RENDER
    else if (context->getRenderType() == renderer::Context::RenderType::DirectXRender)
    {
        renderer::ShaderHeader header;
        header._contentType = renderer::ShaderHeader::ShaderResource::ShaderResource_Source;
        header._shaderModel = renderer::ShaderHeader::ShaderModel::ShaderModel_HLSL_5_1;
        header._optLevel = (flags & ShaderSourceBuildFlag::ShaderSource_OptimisationPerformance) ? 2 : (flags & ShaderSourceBuildFlag::ShaderSource_OptimisationSize) ? 1 : 0;
        header._defines = defines;

        ResourceLoader::registerDecoder(new ShaderHLSLDecoder({ "vs", "ps" }, header, !(flags & ShaderSourceBuildFlag::ShaderSource_DontUseReflection)));
    }
#endif

    ResourceLoader::registerRoot("");
    ResourceLoader::registerRoot("../../../../");

    ResourceLoader::registerPath("");
    ResourceLoader::registerPathes(ResourceLoaderManager::getInstance()->getPathes());
}

ShaderSourceFileLoader::~ShaderSourceFileLoader()
{

}

renderer::Shader * ShaderSourceFileLoader::load(const std::string & name, const std::string & alias)
{
    for (std::string& root : m_roots)
    {
        for (std::string& path : m_pathes)
        {
            const std::string fullPath = root + path + name;
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

                LOG_INFO("ShaderSourceFileLoader::load Shader [%s] is loaded", name.c_str());
                return static_cast<renderer::Shader*>(resource);
            }
        }
    }

    LOG_WARNING("ShaderSourceFileLoader::load: File [%s] decoder file hasn't found", name.c_str());
    return nullptr;
}

} //namespace resource
} //namespace v3d
