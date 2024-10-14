#include "ShaderBinaryFileLoader.h"

#include "Renderer/Shader.h"
#include "Renderer/Device.h"
#include "Stream/FileLoader.h"
#include "Resource/ResourceManager.h"

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

 ShaderBinaryFileLoader::ShaderBinaryFileLoader(const renderer::Device* device, ShaderCompileFlags flags) noexcept
     : m_policy()
     , m_flags(flags)
 {
     if (device->getRenderType() == renderer::Device::RenderType::Vulkan)
     {
#ifdef USE_SPIRV
         m_policy._content = renderer::ShaderContent::Bytecode;
         m_policy._model = renderer::ShaderModel::Default;

         ResourceDecoderRegistration::registerDecoder(V3D_NEW(ShaderSpirVDecoder, memory::MemoryLabel::MemorySystem)({ "vspv", "fspv", "cspv" }));
#else //USE_SPIRV
         ASSERT(false, "not supported");
#endif //USE_SPIRV
         }
     else if (device->getRenderType() == renderer::Device::RenderType::DirectX)
     {
#ifdef D3D_RENDER
         {
             renderer::ShaderHeader header;
             header._contentType = renderer::ShaderHeader::ShaderContent::Bytecode;
             header._shaderModel = renderer::ShaderHeader::ShaderModel::Default;
             header._optLevel = 0;

             ResourceDecoderRegistration::registerDecoder(V3D_NEW(ShaderHLSLDecoder, memory::MemoryLabel::MemorySystem)({ "vspv", "fspv", "cspv" }));
     }
#else //D3D_RENDER
         ASSERT(false, "not supported");
#endif //D3D_RENDER
 }

     ResourceLoader::registerPathes(ResourceManager::getInstance()->getPathes());
 }

ShaderBinaryFileLoader::ShaderBinaryFileLoader(const renderer::Device* device, const ShaderDecoder::ShaderPolicy& policy, ShaderCompileFlags flags) noexcept
    : m_policy(policy)
    , m_flags(flags)
{
    if (device->getRenderType() == renderer::Device::RenderType::Vulkan)
    {
#ifdef USE_SPIRV
        ResourceDecoderRegistration::registerDecoder(V3D_NEW(ShaderSpirVDecoder, memory::MemoryLabel::MemorySystem)());
#else //USE_SPIRV
        ASSERT(false, "not supported");
#endif //USE_SPIRV
    }
    else if (device->getRenderType() == renderer::Device::RenderType::DirectX)
    {
#ifdef D3D_RENDER
        {
            renderer::ShaderHeader header;
            header._contentType = renderer::ShaderHeader::ShaderContent::Bytecode;
            header._shaderModel = renderer::ShaderHeader::ShaderModel::Default;
            header._optLevel = 0;

            ResourceDecoderRegistration::registerDecoder(V3D_NEW(ShaderHLSLDecoder, memory::MemoryLabel::MemorySystem)({ "vspv", "fspv", "cspv" }));
        }
#else //D3D_RENDER
        ASSERT(false, "not supported");
#endif //D3D_RENDER
    }

    ResourceLoader::registerPathes(ResourceManager::getInstance()->getPathes());
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
                Resource* resource = decoder->decode(file, &m_policy, m_flags, name);

                file->close();
                V3D_DELETE(file, memory::MemoryLabel::MemorySystem);
                file = nullptr;

                if (!resource)
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
