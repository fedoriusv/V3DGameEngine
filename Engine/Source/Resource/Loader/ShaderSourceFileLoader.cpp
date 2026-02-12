#include "ShaderSourceFileLoader.h"

#include "Stream/FileLoader.h"
#include "Renderer/Device.h"
#include "Renderer/Shader.h"

#include "Resource/ResourceManager.h"
#include "Resource/Decoder/ShaderDXCDecoder.h"
#ifdef USE_SPIRV
#   include "Resource/Decoder/ShaderSpirVDecoder.h"
#endif //USE_SPIRV
#if D3D_RENDER
#   include "Resource/Decoder/ShaderHLSLDecoder.h"
#endif // D3Dre

namespace v3d
{
namespace resource
{

static std::map<std::string, renderer::ShaderModel> k_extensionList =
{
    { "hlsl", renderer::ShaderModel::HLSL },
    { "glsl", renderer::ShaderModel::GLSL_450 }
};

ShaderSourceFileLoader::ShaderSourceFileLoader(renderer::Device* device, ShaderCompileFlags compileFlags) noexcept
{
    if (device->getRenderType() == renderer::Device::RenderType::Vulkan)
    {
        if (compileFlags & ShaderCompileFlag::ShaderCompile_UseDXCompilerForSpirV)
        {
            ResourceDecoderRegistration::registerDecoder(V3D_NEW(ShaderDXCDecoder, memory::MemoryLabel::MemorySystem)({ "hlsl" }, compileFlags));
        }
#ifdef USE_SPIRV
        else
        {
            ResourceDecoderRegistration::registerDecoder(V3D_NEW(ShaderSpirVDecoder, memory::MemoryLabel::MemorySystem)({ "hlsl", "glsl" }, compileFlags));
        }
#endif //USE_SPIRV
    }
#if D3D_RENDER
        else if (device->getRenderType() == renderer::Device::RenderType::DirectX)
        {
#if !defined(PLATFORM_XBOX)
        if (compileFlags & ShaderCompileFlag::ShaderCompile_UseLegacyCompilerForHLSL)
        {
            ResourceDecoderRegistration::registerDecoder(V3D_NEW(ShaderHLSLDecoder, memory::MemoryLabel::MemorySystem)({ "vs", "ps", "cs" }, header, entrypoint, defines, includes, flags));
        }
        else
#endif //!PLATFORM_XBOX
        {
            ResourceDecoderRegistration::registerDecoder(V3D_NEW(ShaderDXCDecoder, memory::MemoryLabel::MemorySystem)({ "vs", "ps", "cs" }, header, entrypoint, defines, includes, header._shaderModel, flags));
        }
#endif //D3D_RENDER
}

renderer::Shader* ShaderSourceFileLoader::load(const std::string& name, const Resource::LoadPolicy& policy, ShaderCompileFlags flags)
{
    for (std::string& root : m_roots)
    {
        for (std::string& path : m_paths)
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
                // Get shader model from extension
                PolicyType newPolicy(*static_cast<const PolicyType*>(&policy));
                if (flags & ShaderCompileFlag::ShaderCompile_ShaderModelFromExt)
                {
                    auto ext = k_extensionList.find(fileExtension);
                    ASSERT(ext != k_extensionList.cend(), "unknown extention");
                    if (ext != k_extensionList.cend())
                    {
                        newPolicy.shaderModel = ext->second;
                    }
                }
                newPolicy.paths.push_back(root + path);

                Resource* resource = decoder->decode(file, &newPolicy, flags, name);

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
