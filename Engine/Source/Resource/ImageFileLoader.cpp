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
    : m_policy()
    , m_flags(flags)
{
#if USE_STB
    ResourceDecoderRegistration::registerDecoder(V3D_NEW(ImageStbDecoder, memory::MemoryLabel::MemorySystem)({ "jpg", "png", "bmp", "tga" }));
#endif //USE_STB
#if USE_GLI
    ResourceDecoderRegistration::registerDecoder(V3D_NEW(ImageGLiDecoder, memory::MemoryLabel::MemorySystem)({ "ktx", "kmg", "dds" }));
#endif //USE_GLI

    ResourceLoader::registerPaths(ResourceManager::getInstance()->getPaths());
}

resource::Bitmap* ImageFileLoader::load(const std::string& name, const std::string& alias)
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

            Resource* resource = decoder->decode(file, &m_policy, m_flags, name);

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

} //namespace resource
} //namespace v3d
