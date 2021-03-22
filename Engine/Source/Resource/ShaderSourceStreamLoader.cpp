#include "ShaderSourceStreamLoader.h"

#include "Renderer/Context.h"
#include "Stream/FileLoader.h"
#include "Resource/ResourceLoaderManager.h"
#include "Renderer/Shader.h"

#ifdef USE_SPIRV
#   include "ShaderSpirVDecoder.h"
#endif //USE_SPIRV

#ifdef D3D_RENDER
#   include "ShaderHLSLDecoder.h"
#   include "ShaderDXCDecoder.h"
#endif

namespace v3d
{
namespace resource
{

ShaderSourceStreamLoader::ShaderSourceStreamLoader(const renderer::Context* context, const renderer::ShaderHeader* header, const stream::Stream* stream, ShaderSourceBuildFlags flags) noexcept
    : m_stream(stream)
{
    ASSERT(context, "context is nullptr");
    switch (context->getRenderType())
    {
    case renderer::Context::RenderType::VulkanRender:
    {
        ASSERT(header, "nullptr");
#ifdef USE_SPIRV
#   ifdef PLATFORM_WINDOWS
        if (flags & ShaderSource_UseDXCompiler)
        {
            ASSERT(header->_shaderModel != renderer::ShaderHeader::ShaderModel::ShaderModel_GLSL_450, "Works only with HLSL lang");
            ResourceDecoderRegistration::registerDecoder(new ShaderDXCDecoder(*header, renderer::ShaderHeader::ShaderModel::ShaderModel_SpirV, !(flags & ShaderSource_DontUseReflection)));
        }
#   endif //PLATFORM_WINDOWS
        ResourceDecoderRegistration::registerDecoder(new ShaderSpirVDecoder(*header, !(flags & ShaderSource_DontUseReflection)));
#endif //USE_SPIRV
        break;
    }

#ifdef D3D_RENDER
    case renderer::Context::RenderType::DirectXRender:
    {
        ASSERT(header, "nullptr");
        if (flags & ShaderSource_UseDXCompiler)
        {
            ResourceDecoderRegistration::registerDecoder(new ShaderDXCDecoder(*header, header->_shaderModel, !(flags & ShaderSource_DontUseReflection)));
        }
        ResourceDecoderRegistration::registerDecoder(new ShaderHLSLDecoder(*header));

        break;
    }
#endif //D3D_RENDER

    default:
    {
        ASSERT(false, "not implemented");
    }
    }
}

ShaderSourceStreamLoader::~ShaderSourceStreamLoader()
{
    m_stream = nullptr;
}

renderer::Shader* ShaderSourceStreamLoader::load(const std::string& name, const std::string& alias)
{
    if (getDecoders().empty())
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

        if (!resource->load())
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
