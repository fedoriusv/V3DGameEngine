#include "ShaderSourceStreamLoader.h"

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

ShaderSourceStreamLoader::ShaderSourceStreamLoader(const renderer::Context* context, const renderer::ShaderHeader* header, const stream::Stream* stream, bool enableReflection) noexcept
    : m_stream(stream)
{
    ASSERT(context, "context is nullptr");
    if (context->getRenderType() == renderer::Context::RenderType::VulkanRender)
    {
#ifdef USE_SPIRV
        {
            ASSERT(header, "nullptr");
            ResourceLoader::registerDecoder(new ShaderSpirVDecoder( *header, enableReflection));
        }
#else //USE_SPIRV
        ASSERT(false, "not implemented");
#endif //USE_SPIRV
    }
    else
    {
        ASSERT(false, "not implemented");
    }
}

ShaderSourceStreamLoader::~ShaderSourceStreamLoader()
{
    m_stream = nullptr;
}

renderer::Shader * ShaderSourceStreamLoader::load(const std::string & name, const std::string & alias)
{
    if (m_decoders.empty())
    {
        LOG_ERROR("ShaderSourceStreamLoader: Decoder is missing");
        return nullptr;
    }

    if (!m_stream || !m_stream->size())
    {
        return nullptr;
    }

    ResourceDecoder* decoder = m_decoders.front();
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
