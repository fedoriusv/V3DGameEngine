#include "ShaderSourceStreamLoader.h"

#include "Renderer/Core/Context.h"
#include "Stream/FileLoader.h"
#include "Resource/ResourceLoaderManager.h"
#include "Renderer/Shader.h"

#ifdef USE_SPIRV
#   include "ShaderSpirVDecoder.h"
#endif //USE_SPIRV

#ifdef D3D_RENDER
#   include "ShaderHLSLDecoder.h"
#   include "ShaderDXCDecoder.h"
#endif //D3D_RENDER

namespace v3d
{
namespace resource
{

ShaderSourceStreamLoader::ShaderSourceStreamLoader(const renderer::Context* context, const renderer::ShaderHeader* header, const stream::Stream* stream,
    const std::string& entrypoint, const renderer::Shader::DefineList& defines, const std::vector<std::string>& includes, ShaderSourceFlags flags) noexcept
    : m_stream(stream)
{
    ASSERT(context, "context is nullptr");
    ASSERT(!(flags & renderer::ShaderCompileFlag::ShaderSource_OptimizationFull) && !(flags & renderer::ShaderCompileFlag::ShaderSource_OptimizationPerformance) ||
        !(flags & renderer::ShaderCompileFlag::ShaderSource_OptimizationSize), "set inside header");

    switch (context->getRenderType())
    {
#ifdef USE_SPIRV
    case renderer::Context::RenderType::VulkanRender:
    {
        ASSERT(header, "nullptr");
#ifdef PLATFORM_WINDOWS
        if (flags & renderer::ShaderCompileFlag::ShaderSource_UseDXCompilerForSpirV)
        {
            ASSERT(header->_shaderModel != renderer::ShaderHeader::ShaderModel::GLSL_450, "Works only with HLSL lang");
            ResourceDecoderRegistration::registerDecoder(
                V3D_NEW(ShaderDXCDecoder, memory::MemoryLabel::MemorySystem)(*header, entrypoint, defines, includes, renderer::ShaderHeader::ShaderModel::SpirV, flags)
            );
        }
        else
#endif //PLATFORM_WINDOWS
        {
            ResourceDecoderRegistration::registerDecoder(
                V3D_NEW(ShaderSpirVDecoder, memory::MemoryLabel::MemorySystem)(*header, entrypoint, defines, includes, flags)
            );
        }

        break;
#endif //USE_SPIRV
    }

#ifdef D3D_RENDER
    case renderer::Context::RenderType::DirectXRender:
    {
        ASSERT(header, "nullptr");
        if (flags & renderer::ShaderCompileFlag::ShaderSource_UseLegacyCompilerForHLSL)
        {
            ResourceDecoderRegistration::registerDecoder(
                V3D_NEW(ShaderHLSLDecoder, memory::MemoryLabel::MemorySystem)(*header, entrypoint, defines, includes, flags)
            );
        }
        else
        {
            ResourceDecoderRegistration::registerDecoder(
                V3D_NEW(ShaderDXCDecoder, memory::MemoryLabel::MemorySystem)(*header, entrypoint, defines, includes, header->_shaderModel, flags)
            );
        }

        break;
    }
#endif //D3D_RENDER

    default:
        ASSERT(false, "not suppported");
    };
}

renderer::Shader* ShaderSourceStreamLoader::load(const std::string& name, const std::string& alias)
{
    if (ShaderSourceStreamLoader::getDecoders().empty())
    {
        LOG_ERROR("ShaderSourceStreamLoader: Decoder is missing");
        return nullptr;
    }

    if (!m_stream || !m_stream->size())
    {
        return nullptr;
    }

    const ResourceDecoder* decoder = getDecoders().front();
    if (decoder)
    {
        Resource* resource = decoder->decode(m_stream, name);
        if (!resource)
        {
            LOG_ERROR("ShaderSourceStreamLoader: Streaming error read stream [%s]", name.c_str());
            return nullptr;
        }

        LOG_INFO("ShaderSourceStreamLoader::load Shader [%s] is loaded", name.c_str());
        return static_cast<renderer::Shader*>(resource);
    }

    LOG_ERROR("ShaderSourceStreamLoader: decoder doesn't exist [%s]", name.c_str());
    return nullptr;
}

} //namespace resource
} //namespace v3d
