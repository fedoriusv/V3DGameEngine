#include "ShaderSourceStreamLoader.h"

#include "Renderer/Device.h"
#include "Renderer/Shader.h"
#include "Stream/FileLoader.h"

#include "Resource/Decoder/ShaderDXCDecoder.h"
#ifdef USE_SPIRV
#   include "Resource/Decoder/ShaderSpirVDecoder.h"
#endif //USE_SPIRV
#ifdef D3D_RENDER
#   include "Resource/Decoder/ShaderHLSLDecoder.h"
#endif //D3D_RENDER

namespace v3d
{
namespace resource
{

ShaderSourceStreamLoader::ShaderSourceStreamLoader(renderer::Device* device, const stream::Stream* stream, ShaderCompileFlags flags) noexcept
    : m_stream(stream)
{
    switch (device->getRenderType())
    {
#ifdef USE_SPIRV
    case renderer::Device::RenderType::Vulkan:
    {
        if (flags & ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV)
        {
            ResourceDecoderRegistration::registerDecoder(V3D_NEW(ShaderDXCDecoder, memory::MemoryLabel::MemorySystem)(flags));
        }
        else
        {
            ResourceDecoderRegistration::registerDecoder(V3D_NEW(ShaderSpirVDecoder, memory::MemoryLabel::MemorySystem)(flags));
        }

        break;
    }
#endif //USE_SPIRV
#ifdef D3D_RENDER
    case renderer::Context::RenderType::DirectX:
    {
        if (flags & ShaderCompileFlag::ShaderCompile_UseLegacyCompilerForHLSL)
        {
            ResourceDecoderRegistration::registerDecoder(V3D_NEW(ShaderHLSLDecoder, memory::MemoryLabel::MemorySystem)();
        }
        else
        {
            ResourceDecoderRegistration::registerDecoder(V3D_NEW(ShaderDXCDecoder, memory::MemoryLabel::MemorySystem)();
        }

        break;
    }
#endif //D3D_RENDER

    default:
        ASSERT(false, "not suppported");
    };
}

renderer::Shader* ShaderSourceStreamLoader::load(const std::string& name, const Resource::LoadPolicy& policy, ShaderCompileFlags flags)
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
        Resource* resource = decoder->decode(m_stream, &policy, flags, name);
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
