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

ShaderSourceFileLoader::ShaderSourceFileLoader(const renderer::Context* context, const std::string& entrypoint, const renderer::Shader::DefineList& defines,
    const std::vector<std::string>& includes, ShaderSourceFlags flags) noexcept
{
    ASSERT(context, "context is nullptr");

    u32 optimizationLevel = 0;
    if (flags & renderer::ShaderCompileFlag::ShaderSource_OptimizationFull)
    {
        optimizationLevel = 3;
    }
    else if (flags & renderer::ShaderCompileFlag::ShaderSource_OptimizationPerformance)
    {
        optimizationLevel = 2;
    }
    else if (flags & renderer::ShaderCompileFlag::ShaderSource_OptimizationSize)
    {
        optimizationLevel = 1;
    }

    if (context->getRenderType() == renderer::Context::RenderType::VulkanRender)
    {
#ifdef USE_SPIRV
        {
            ASSERT(!(flags & renderer::ShaderCompileFlag::ShaderSource_OptimizationSize || flags & renderer::ShaderCompileFlag::ShaderSource_OptimizationPerformance ||
                flags & renderer::ShaderCompileFlag::ShaderSource_OptimizationFull), "define can't work with the flag because optimization removes names from spirv");

            renderer::ShaderHeader header;
            header._contentType = renderer::ShaderHeader::ShaderContent::Source;
            header._shaderModel = renderer::ShaderHeader::ShaderModel::GLSL_450;
            header._optLevel = optimizationLevel;

            ResourceDecoderRegistration::registerDecoder(
                V3D_NEW(ShaderSpirVDecoder, memory::MemoryLabel::MemorySystem)({ "vert", "frag", "comp" }, header, entrypoint, defines, includes, flags)
            );
        }

        {
            renderer::ShaderHeader header;
            header._contentType = renderer::ShaderHeader::ShaderContent::Source;
            header._shaderModel = renderer::ShaderHeader::ShaderModel::HLSL_5_1;
            header._optLevel = optimizationLevel;

            ResourceDecoderRegistration::registerDecoder(
                V3D_NEW(ShaderSpirVDecoder, memory::MemoryLabel::MemorySystem)({ "vs", "ps", "cs" }, header, entrypoint, defines, includes, flags)
            );
        }
#endif //USE_SPIRV
    }

#if D3D_RENDER
    else if (context->getRenderType() == renderer::Context::RenderType::DirectXRender)
    {
#if !defined(PLATFORM_XBOX)
        if (flags & renderer::ShaderCompileFlag::ShaderSource_UseLegacyCompilerForHLSL)
        {
            renderer::ShaderHeader header;
            header._contentType = renderer::ShaderHeader::ShaderContent::Source;
            header._shaderModel = renderer::ShaderHeader::ShaderModel::HLSL_5_1;
            header._optLevel = optimizationLevel;

            ResourceDecoderRegistration::registerDecoder(
                V3D_NEW(ShaderHLSLDecoder, memory::MemoryLabel::MemorySystem)({ "vs", "ps", "cs" }, header, entrypoint, defines, includes, flags)
            );
        }
        else
#endif //!PLATFORM_XBOX
        {
            renderer::ShaderHeader header;
            header._contentType = renderer::ShaderHeader::ShaderContent::Source;
            header._shaderModel = renderer::ShaderHeader::ShaderModel::HLSL_6_1;
            header._optLevel = optimizationLevel;

            ResourceDecoderRegistration::registerDecoder(
                V3D_NEW(ShaderDXCDecoder, memory::MemoryLabel::MemorySystem)({ "vs", "ps", "cs" }, header, entrypoint, defines, includes, header._shaderModel, flags)
            );
        }
    }
#endif //D3D_RENDER

    ResourceLoader::registerRoot("");
    ResourceLoader::registerRoot("../../../../");

    ResourceLoader::registerPath("");
    ResourceLoader::registerPathes(ResourceLoaderManager::getInstance()->getPathes());
}

ShaderSourceFileLoader::ShaderSourceFileLoader(const renderer::Context* context, renderer::ShaderType type, const std::string& entrypoint,
    const std::vector<std::pair<std::string, std::string>>& defines, const std::vector<std::string>& includes, ShaderSourceFlags flags) noexcept
{
    ASSERT(context, "context is nullptr");

    u32 optimizationLevel = 0;
    if (flags & renderer::ShaderCompileFlag::ShaderSource_OptimizationFull)
    {
        optimizationLevel = 3;
    }
    else if (flags & renderer::ShaderCompileFlag::ShaderSource_OptimizationPerformance)
    {
        optimizationLevel = 2;
    }
    else if (flags & renderer::ShaderCompileFlag::ShaderSource_OptimizationSize)
    {
        optimizationLevel = 1;
    }

    if (context->getRenderType() == renderer::Context::RenderType::VulkanRender)
    {
        if (flags & renderer::ShaderCompileFlag::ShaderSource_UseDXCompilerForSpirV)
        {
            renderer::ShaderHeader header;
            header._contentType = renderer::ShaderHeader::ShaderContent::Source;
            header._shaderModel = renderer::ShaderHeader::ShaderModel::HLSL_6_1;
            header._shaderType = type;
            header._optLevel = optimizationLevel;

            ResourceDecoderRegistration::registerDecoder(
                V3D_NEW(ShaderDXCDecoder, memory::MemoryLabel::MemorySystem)({ "hlsl" }, header, entrypoint, defines, includes, renderer::ShaderHeader::ShaderModel::SpirV, flags)
            );
        }
        else
        {
            renderer::ShaderHeader header;
            header._contentType = renderer::ShaderHeader::ShaderContent::Source;
            header._shaderModel = renderer::ShaderHeader::ShaderModel::HLSL_6_1;
            header._shaderType = type;
            header._optLevel = optimizationLevel;

            ResourceDecoderRegistration::registerDecoder(
                V3D_NEW(ShaderSpirVDecoder, memory::MemoryLabel::MemorySystem)({ "hlsl" }, header, entrypoint, defines, includes, flags)
            );
        }
    }
#if D3D_RENDER
    else if (context->getRenderType() == renderer::Context::RenderType::DirectXRender)
    {
#if !defined(PLATFORM_XBOX)
        if (flags & renderer::ShaderCompileFlag::ShaderSource_UseLegacyCompilerForHLSL)
        {
            renderer::ShaderHeader header;
            header._contentType = renderer::ShaderHeader::ShaderContent::Source;
            header._shaderModel = renderer::ShaderHeader::ShaderModel::HLSL_5_1;
            header._shaderType = type;
            header._optLevel = optimizationLevel;

            ResourceDecoderRegistration::registerDecoder(
                V3D_NEW(ShaderHLSLDecoder, memory::MemoryLabel::MemorySystem)({ "hlsl" }, header, entrypoint, defines, includes, flags)
            );
    }
        else
#endif //!PLATFORM_XBOX
        {
            renderer::ShaderHeader header;
            header._contentType = renderer::ShaderHeader::ShaderContent::Source;
            header._shaderModel = renderer::ShaderHeader::ShaderModel::HLSL_6_1;
            header._shaderType = type;
            header._optLevel = optimizationLevel;

            ResourceDecoderRegistration::registerDecoder(
                V3D_NEW(ShaderDXCDecoder, memory::MemoryLabel::MemorySystem)({ "hlsl" }, header, entrypoint, defines, includes, header._shaderModel, flags)
            );
        }
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
                V3D_DELETE(file, memory::MemoryLabel::MemorySystem);
                file = nullptr;

                if (!resource)
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
