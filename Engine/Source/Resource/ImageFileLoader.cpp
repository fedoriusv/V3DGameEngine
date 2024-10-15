#include "ImageFileLoader.h"

#include "Renderer/Device.h"
#include "Resource/ResourceManager.h"
#include "Stream/FileLoader.h"
#include "Bitmap.h"

#if USE_STB
#   include "ImageStbDecoder.h"
#endif //USE_STB

#if USE_GLI
#   include "ImageGLiDecoder.h"
#endif //USE_GLI

namespace v3d
{
namespace resource
{

ImageFileLoader::ImageFileLoader(ImageLoaderFlags flags) noexcept
{
#if USE_STB
    {
        resource::BitmapHeader header;
        ResourceDecoderRegistration::registerDecoder(V3D_NEW(ImageStbDecoder, memory::MemoryLabel::MemorySystem)({ "jpg", "png", "bmp", "tga" }, header, flags));
    }
#endif //USE_STB

#if USE_GLI
    {
        resource::BitmapHeader header;
        ResourceDecoderRegistration::registerDecoder(V3D_NEW(ImageGLiDecoder, memory::MemoryLabel::MemorySystem)({ "ktx", "kmg", "dds" }, header, flags));
    }
#endif //USE_GLI

    ResourceLoader::registerPathes(ResourceManager::getInstance()->getPathes());
}

resource::Bitmap* ImageFileLoader::load(const std::string& name, const std::string& alias)
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
            if (!decoder)
            {
                LOG_WARNING("ImageFileLoader::load: File [%s] decoder hasn't found", name.c_str());
                return nullptr;
            }

            Resource* resource = decoder->decode(file, nullptr, 0, name);

            file->close();
            V3D_DELETE(file, memory::MemoryLabel::MemorySystem);
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

} //namespace resource
} //namespace v3d
