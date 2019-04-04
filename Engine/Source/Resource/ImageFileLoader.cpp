#include "ImageFileLoader.h"

#include "Renderer/Context.h"
#include "Stream/FileLoader.h"
#include "Resource/ResourceLoaderManager.h"
#include "Resource/Image.h"
#if USE_STB
#   include "ImageStbDecoder.h"
#endif USE_STB

#if USE_GLI
#   include "ImageGLiDecoder.h"
#endif //USE_GLI

namespace v3d
{
namespace resource
{

ImageFileLoader::ImageFileLoader(u32 flags) noexcept
{
#if USE_STB
    {
        resource::ImageHeader header;
        ResourceLoader::registerDecoder(new ImageStbDecoder({ "jpg", "png", "bmp", "tga" }, header, false));
    }
#endif USE_STB

#if USE_GLI
    {
        resource::ImageHeader header;
        ResourceLoader::registerDecoder(new ImageGLiDecoder({ "ktx", "kmg", "dds" }, header, false));
    }
#endif //USE_GLI

    ResourceLoader::registerPath("../../../../");
    ResourceLoader::registerPath("../../../../../");
    ResourceLoader::registerPath("../../../../engine/");
    ResourceLoader::registerPathes(ResourceLoaderManager::getInstance()->getPathes());
}

ImageFileLoader::~ImageFileLoader()
{
}

resource::Image* ImageFileLoader::load(const std::string & name, const std::string & alias)
{
    for (std::string& path : m_pathes)
    {
        const std::string fullPath = path + name;
        stream::Stream* file = stream::FileLoader::load(fullPath);
        if (!file)
        {
            continue;
        }

        std::string fileExtension = stream::FileLoader::getFileExtension(name);
        ResourceDecoder* decoder = ResourceLoader::findDecoder(fileExtension);
        if (decoder)
        {
           Resource* resource = decoder->decode(file, name);
           file->close();

           delete file;

           if (!resource)
           {
               LOG_ERROR("ImageFileLoader: Streaming error read file [%s]", name.c_str());
               return nullptr;
           }

           if (!resource->load())
           {
               LOG_ERROR("ImageFileLoader: Streaming error read file [%s]", name.c_str());
               return nullptr;
           }

           LOG_INFO("ImageFileLoader::load Shader [%s] is loaded", name.c_str());
           return static_cast<resource::Image*>(resource);
        }
    }

    LOG_WARNING("ImageFileLoader::load: File [%s] not found", name.c_str());
    return nullptr;
}

} //namespace resource
} //namespace v3d
