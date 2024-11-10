#include "ShaderSourceFileLoader.h"

#include "Stream/FileLoader.h"
#include "Renderer/Device.h"
#include "Renderer/Shader.h"
#include "ResourceManager.h"

#include "ShaderDXCDecoder.h"
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

ShaderSourceFileLoader::ShaderSourceFileLoader(renderer::Device* device, const ShaderDecoder::ShaderPolicy& policy, ShaderCompileFlags flags) noexcept
    : m_policy(policy)
    , m_flags(flags)
{
    switch (device->getRenderType())
    {
    case renderer::Device::RenderType::Vulkan:
#ifdef USE_SPIRV
        ResourceDecoderRegistration::registerDecoder(V3D_NEW(ShaderSpirVDecoder, memory::MemoryLabel::MemorySystem)());
        break;
#endif //USE_SPIRV

#if D3D_RENDER
    case renderer::Device::RenderType::Vulkan:
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

        break;
    }
#endif //D3D_RENDER
    default:
        ASSERT(false, "not suppported");
    }

    ResourceLoader::registerPathes(ResourceManager::getInstance()->getPathes());
}

ShaderSourceFileLoader::ShaderSourceFileLoader(renderer::Device* device, renderer::ShaderType type, const std::string& entryPoint,
    const std::vector<std::pair<std::string, std::string>>& defines, const std::vector<std::string>& includes, ShaderCompileFlags flags) noexcept
    : m_flags(flags)
{
    if (device->getRenderType() == renderer::Device::RenderType::Vulkan)
    {
        m_policy._content = renderer::ShaderContent::Source;
        m_policy._shaderModel = (flags & ShaderCompileFlag::ShaderCompile_UseLegacyCompilerForHLSL) ? renderer::ShaderModel::HLSL_5_1 : renderer::ShaderModel::HLSL;
        m_policy._type = type;
        m_policy._defines = defines;
        m_policy._includes = includes;
        m_policy._entryPoint = entryPoint;

        if (flags & ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV)
        {
            ResourceDecoderRegistration::registerDecoder(V3D_NEW(ShaderDXCDecoder, memory::MemoryLabel::MemorySystem)({ "hlsl" }));
        }
        else
        {
            ResourceDecoderRegistration::registerDecoder(V3D_NEW(ShaderSpirVDecoder, memory::MemoryLabel::MemorySystem)({ "hlsl" }));
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

    ResourceLoader::registerPathes(ResourceManager::getInstance()->getPathes());
}

renderer::Shader* ShaderSourceFileLoader::load(const std::string& name, const std::string& alias)
{
    for (std::string& root : m_roots)
    {
        for (std::string& path : m_pathes)
        {
            const std::string fullPath = root + path + name;
            stream::FileStream* file = stream::FileLoader::load(fullPath);
            if (!file)
            {
                continue;
            }

            std::string fileExtension = stream::FileLoader::getFileExtension(name);
            const ResourceDecoder* decoder = findDecoder(fileExtension);
            if (decoder)
            {
                Resource* resource = decoder->decode(file, &m_policy, m_flags, name);

                stream::FileLoader::close(file);
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
