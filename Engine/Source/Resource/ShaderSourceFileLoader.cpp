#include "ShaderSourceFileLoader.h"

#include "Renderer/Core/Context.h"
#include "Stream/FileLoader.h"
#include "Resource/ResourceLoaderManager.h"
#include "Renderer/Shader.h"

#ifdef USE_SPIRV
#   include "ShaderSpirVDecoder.h"
#endif //USE_SPIRV

#if D3D_RENDER
#   include "ShaderHLSLDecoder.h"
#   include "ShaderDXCDecoder.h"
#endif // D3Dre


namespace v3d
{
namespace resource
{

ShaderSourceFileLoader::ShaderSourceFileLoader(const renderer::Context* context, const std::vector<std::pair<std::string, std::string>>& defines, ShaderSourceBuildFlags flags) noexcept
{
    ASSERT(context, "context is nullptr");

    u32 optimizationLevel = 0;
    if (flags & ShaderSourceBuildFlag::ShaderSource_OptimizationFull)
    {
        optimizationLevel = 3;
    }
    else if (flags & ShaderSourceBuildFlag::ShaderSource_OptimizationPerformance)
    {
        optimizationLevel = 2;
    }
    else if (flags & ShaderSourceBuildFlag::ShaderSource_OptimizationSize)
    {
        optimizationLevel = 1;
    }

    if (context->getRenderType() == renderer::Context::RenderType::VulkanRender)
    {
#ifdef USE_SPIRV
        {
#if USE_STRING_ID_SHADER
            ASSERT(!(flags & ShaderSourceBuildFlag::ShaderSource_OptimizationSize || flags & ShaderSourceBuildFlag::ShaderSource_OptimizationPerformance || flags & ShaderSourceBuildFlag::ShaderSource_OptimizationFull), "define can't work with the flag because optimization removes names from spirv");
#endif
            renderer::ShaderHeader header;
            header._contentType = renderer::ShaderHeader::ShaderResource::Source;
            header._shaderModel = renderer::ShaderHeader::ShaderModel::GLSL_450;
            header._optLevel = optimizationLevel;
            header._defines = defines;
            header._extraFlags |= (flags & ShaderSourceBuildFlag::ShaderSource_Patched) ? ShaderSourceBuildFlag::ShaderSource_Patched : header._extraFlags;

            ResourceDecoderRegistration::registerDecoder(new ShaderSpirVDecoder( { "vert", "frag", "comp" }, header, !(flags & ShaderSourceBuildFlag::ShaderSource_DontUseReflection) ));
        }

        {
            renderer::ShaderHeader header;
            header._contentType = renderer::ShaderHeader::ShaderResource::Source;
            header._shaderModel = renderer::ShaderHeader::ShaderModel::HLSL_5_1;
            header._optLevel = optimizationLevel;
            header._defines = defines;
            header._extraFlags |= (flags & ShaderSourceBuildFlag::ShaderSource_Patched) ? ShaderSourceBuildFlag::ShaderSource_Patched : header._extraFlags;

#ifdef PLATFORM_WINDOWS
            /*if (flags & ShaderSourceBuildFlag::ShaderSource_UseDXCompiler)
            {
                ASSERT(false, "assamble and reflaction is not supported now for spirv");
                header._shaderModel = renderer::ShaderHeader::ShaderModel::ShaderModel_HLSL_6_1;
                ResourceDecoderRegistration::registerDecoder(new ShaderDXCDecoder({ "vs", "ps" }, header, renderer::ShaderHeader::ShaderModel::ShaderModel_SpirV, !(flags & ShaderSourceBuildFlag::ShaderSource_DontUseReflection)));
            }*/
#endif //PLATFORM_WINDOWS
            ResourceDecoderRegistration::registerDecoder(new ShaderSpirVDecoder( { "vs", "ps", "cs" }, header, !(flags & ShaderSourceBuildFlag::ShaderSource_DontUseReflection) ));
        }
#else //USE_SPIRV
        ASSERT(false, "not implemented");
#endif //USE_SPIRV
    }

#if D3D_RENDER
    else if (context->getRenderType() == renderer::Context::RenderType::DirectXRender)
    {
        renderer::ShaderHeader header;
        header._contentType = renderer::ShaderHeader::ShaderResource::Source;
        header._shaderModel = renderer::ShaderHeader::ShaderModel::HLSL_5_1;
        header._optLevel = optimizationLevel;
        header._defines = defines;

#if defined(PLATFORM_XBOX)
        flags |= ShaderSource_UseDXCompiler;
#endif
        if (flags & ShaderSourceBuildFlag::ShaderSource_UseDXCompiler)
        {
            header._shaderModel = renderer::ShaderHeader::ShaderModel::HLSL_6_1;
            ResourceDecoderRegistration::registerDecoder(new ShaderDXCDecoder({ "vs", "ps" }, header, header._shaderModel, !(flags & ShaderSourceBuildFlag::ShaderSource_DontUseReflection)));
        }
        ResourceDecoderRegistration::registerDecoder(new ShaderHLSLDecoder({ "vs", "ps", "cs" }, header, !(flags & ShaderSourceBuildFlag::ShaderSource_DontUseReflection)));
    }
#endif

    ResourceLoader::registerRoot("");
    ResourceLoader::registerRoot("../../../../");

    ResourceLoader::registerPath("");
    ResourceLoader::registerPathes(ResourceLoaderManager::getInstance()->getPathes());
}

ShaderSourceFileLoader::ShaderSourceFileLoader(const renderer::Context* context, renderer::ShaderType type, const std::string& entryPoint, const std::vector<std::pair<std::string, std::string>>& defines, ShaderSourceBuildFlags flags) noexcept
{
    ASSERT(context, "context is nullptr");

    renderer::ShaderHeader header(type);
    header._contentType = renderer::ShaderHeader::ShaderResource::Source;
    header._shaderModel = renderer::ShaderHeader::ShaderModel::Default;
    header._defines = defines;
    header._entryPoint = entryPoint;
    header._optLevel = 0;
    if (flags & ShaderSourceBuildFlag::ShaderSource_OptimizationFull)
    {
        header._optLevel = 3;
    }
    else if (flags & ShaderSourceBuildFlag::ShaderSource_OptimizationPerformance)
    {
        header._optLevel = 2;
    }
    else if (flags & ShaderSourceBuildFlag::ShaderSource_OptimizationSize)
    {
        header._optLevel = 1;
    }

    if (context->getRenderType() == renderer::Context::RenderType::VulkanRender)
    {
        header._optLevel = (flags & ShaderSourceBuildFlag::ShaderSource_OptimizationPerformance) ? 2 : (flags & ShaderSourceBuildFlag::ShaderSource_OptimizationSize) ? 1 : 0;
#ifdef USE_SPIRV
        /*if (flags & ShaderSourceBuildFlag::ShaderSource_UseDXCompiler)
        {
            ASSERT(false, "assamble and reflaction is not supported now for spirv");
            header._shaderModel = renderer::ShaderHeader::ShaderModel::ShaderModel_HLSL_6_1;
            ResourceDecoderRegistration::registerDecoder(new ShaderDXCDecoder({ "hlsl" }, header, renderer::ShaderHeader::ShaderModel::ShaderModel_SpirV, !(flags & ShaderSourceBuildFlag::ShaderSource_DontUseReflection)));
        }*/
        header._shaderModel = renderer::ShaderHeader::ShaderModel::HLSL_5_1;
        ResourceDecoderRegistration::registerDecoder(new ShaderSpirVDecoder({ "hlsl" }, header, !(flags & ShaderSourceBuildFlag::ShaderSource_DontUseReflection)));

        header._shaderModel = renderer::ShaderHeader::ShaderModel::GLSL_450;
        ResourceDecoderRegistration::registerDecoder(new ShaderSpirVDecoder({ "vert", "frag", "comp" }, header, !(flags & ShaderSourceBuildFlag::ShaderSource_DontUseReflection)));
#endif
    }
#if D3D_RENDER
    else if (context->getRenderType() == renderer::Context::RenderType::DirectXRender)
    {
        header._shaderModel = renderer::ShaderHeader::ShaderModel::HLSL_5_1;

#if defined(PLATFORM_XBOX)
        flags |= ShaderSource_UseDXCompiler;
#endif
        if (flags & ShaderSourceBuildFlag::ShaderSource_UseDXCompiler)
        {
            header._shaderModel = renderer::ShaderHeader::ShaderModel::HLSL_6_1;
            ResourceDecoderRegistration::registerDecoder(new ShaderDXCDecoder({ "hlsl" }, header, header._shaderModel, !(flags & ShaderSourceBuildFlag::ShaderSource_DontUseReflection)));
        }
        ResourceDecoderRegistration::registerDecoder(new ShaderHLSLDecoder({ "hlsl" }, header, !(flags & ShaderSourceBuildFlag::ShaderSource_DontUseReflection)));
    }
#endif

    ResourceLoader::registerRoot("");
    ResourceLoader::registerRoot("../../../../");

    ResourceLoader::registerPath("");
    ResourceLoader::registerPathes(ResourceLoaderManager::getInstance()->getPathes());
}

renderer::Shader* ShaderSourceFileLoader::load(const std::string& name, const std::string& alias)
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

    LOG_WARNING("ShaderSourceFileLoader::load: File [%s] decoder or file hasn't found", name.c_str());
    return nullptr;
}

} //namespace resource
} //namespace v3d
