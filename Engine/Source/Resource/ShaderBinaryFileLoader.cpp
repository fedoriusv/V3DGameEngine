#include "ShaderBinaryFileLoader.h"

#include "Renderer/Shader.h"
#include "Renderer/Core/Context.h"
#include "Stream/FileLoader.h"
#include "Resource/ResourceLoaderManager.h"
#ifdef USE_SPIRV
#   include "ShaderSpirVDecoder.h"
#endif //USE_SPIRV

#if D3D_RENDER
#   include "ShaderHLSLDecoder.h"
#endif //D3D_RENDER

namespace v3d
{
namespace resource
{

ShaderBinaryFileLoader::ShaderBinaryFileLoader(const renderer::Context* context, const std::vector<std::pair<std::string, std::string>>& defines, ShaderBinaryBuildFlags flags) noexcept
{
    ASSERT(context, "context is nullptr");
    if (context->getRenderType() == renderer::Context::RenderType::VulkanRender)
    {
#ifdef USE_SPIRV
        {
            renderer::ShaderHeader header;
            header._contentType = renderer::ShaderHeader::ShaderResource::Bytecode;
            header._shaderModel = renderer::ShaderHeader::ShaderModel::SpirV;
            header._optLevel = 0;
            ASSERT(defines.empty(), "cant use defines, should be finaly compiled already");

            ResourceDecoderRegistration::registerDecoder(new ShaderSpirVDecoder( { "vspv", "fspv", "cspv" }, header, !(flags & ShaderBinaryBuildFlag::ShaderBinary_DontUseReflaction) ));
        }
#else //USE_SPIRV
        ASSERT(false, "not implemented");
#endif //USE_SPIRV
    }
    else if (context->getRenderType() == renderer::Context::RenderType::DirectXRender)
    {
#ifdef D3D_RENDER
        {
            renderer::ShaderHeader header;
            header._contentType = renderer::ShaderHeader::ShaderResource::Bytecode;
            header._shaderModel = renderer::ShaderHeader::ShaderModel::Default;
            header._optLevel = 0;
            ASSERT(defines.empty(), "cant use defines, should be finaly compiled already");

            ResourceDecoderRegistration::registerDecoder(new ShaderHLSLDecoder({ "vsb", "psb", "csb" }, header, !(flags & ShaderBinaryBuildFlag::ShaderBinary_DontUseReflaction)));
        }
#else //D3D_RENDER
        ASSERT(false, "not implemented");
#endif //D3D_RENDER
    }

    ResourceLoader::registerRoot("");
    ResourceLoader::registerRoot("../../../../");

    ResourceLoader::registerPath("");
    ResourceLoader::registerPathes(ResourceLoaderManager::getInstance()->getPathes());
}

renderer::Shader* ShaderBinaryFileLoader::load(const std::string& name, const std::string& alias)
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
            const ResourceDecoder* decoder = findDecoder(fileExtension);
            if (decoder)
            {
                Resource* resource = decoder->decode(file, name);
                file->close();

                delete file;

                if (!resource)
                {
                    LOG_ERROR("ShaderBinaryFileLoader: Streaming error read file [%s]", name.c_str());
                    return nullptr;
                }

                if (!resource->load())
                {
                    LOG_ERROR("ShaderBinaryFileLoader: Streaming error read file [%s]", name.c_str());
                    return nullptr;
                }

                LOG_INFO("ShaderBinaryFileLoader::load Shader [%s] is loaded", name.c_str());
                return static_cast<renderer::Shader*>(resource);
            }
        }
    }

    LOG_WARNING("ShaderBinaryFileLoader::load: File [%s] decoder hasn't found", name.c_str());
    return nullptr;
}

} //namespace resource
} //namespace v3d
