#include "ImageFileLoader.h"

#include "Renderer/Device.h"
#include "Stream/FileLoader.h"
#include "Resource/Bitmap.h"

#include "Resource/ResourceManager.h"
#if USE_STB
#   include "Resource/Decoder/ImageStbDecoder.h"
#endif //USE_STB

#if USE_GLI
#   include "Resource/Decoder/ImageGLiDecoder.h"
#endif //USE_GLI

namespace v3d
{
namespace resource
{

static std::vector<std::string> k_bitmapSupportedFormats = { "jpg", "png", "bmp", "tga", "hdr" };
static std::vector<std::string> k_textureSupportedFormats = { "ktx", "kmg", "dds" };

BitmapFileLoader::BitmapFileLoader() noexcept
{
#if USE_STB
    ResourceDecoderRegistration::registerDecoder(V3D_NEW(BitmapStbDecoder, memory::MemoryLabel::MemorySystem)(k_bitmapSupportedFormats));
#endif //USE_STB
#if USE_GLI
    ResourceDecoderRegistration::registerDecoder(V3D_NEW(BitmapGLiDecoder, memory::MemoryLabel::MemorySystem)(k_textureSupportedFormats));
#endif //USE_GLI
}

resource::Bitmap* BitmapFileLoader::load(const std::string& name, const Resource::LoadPolicy& policy, u32 flags)
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
            if (!decoder)
            {
                LOG_WARNING("ImageFileLoader::load: File [%s] decoder hasn't found", name.c_str());
                return nullptr;
            }

            Resource* resource = decoder->decode(file, &policy, flags, name);

            stream::FileLoader::close(file);
            file = nullptr;

            if (!resource)
            {
                LOG_ERROR("ImageFileLoader: Streaming error read file [%s]", name.c_str());
                return nullptr;
            }

            LOG_INFO("ImageFileLoader::load Image [%s] is loaded", name.c_str());
            return static_cast<resource::Bitmap*>(resource);
        }
    }

    LOG_WARNING("ImageFileLoader::load: File [%s] hasn't found", name.c_str());
    return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

TextureFileLoader::TextureFileLoader(renderer::Device* device) noexcept
{
#if USE_STB
    ResourceDecoderRegistration::registerDecoder(V3D_NEW(TextureStbDecoder, memory::MemoryLabel::MemorySystem)(device, k_bitmapSupportedFormats));
#endif //USE_STB
#if USE_GLI
    ResourceDecoderRegistration::registerDecoder(V3D_NEW(TextureGLiDecoder, memory::MemoryLabel::MemorySystem)(device, k_textureSupportedFormats));
#endif //USE_GLI
}

renderer::Texture* TextureFileLoader::load(const std::string& name, const Resource::LoadPolicy& policy, u32 flags)
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
            if (!decoder)
            {
                LOG_WARNING("TextureFileLoader::load: File [%s] decoder hasn't found", name.c_str());
                return nullptr;
            }

            Resource* resource = decoder->decode(file, &policy, flags, name);

            stream::FileLoader::close(file);
            file = nullptr;

            if (!resource)
            {
                LOG_ERROR("TextureFileLoader: Streaming error read file [%s]", name.c_str());
                return nullptr;
            }

            LOG_INFO("TextureFileLoader::load Image [%s] is loaded", name.c_str());
            return static_cast<renderer::Texture*>(resource);
        }
    }

    LOG_WARNING("TextureFileLoader::load: File [%s] hasn't found", name.c_str());
    return nullptr;
}

} //namespace resource
} //namespace v3d
