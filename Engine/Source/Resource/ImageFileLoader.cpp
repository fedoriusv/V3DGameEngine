#include "ImageFileLoader.h"

#include "Resource/Image.h"
#include "ImageStbDecoder.h"

#include "Stream/FileLoader.h"
#include "Renderer/Context.h"

namespace v3d
{
namespace resource
{

ImageFileLoader::ImageFileLoader(const renderer::Context* context) noexcept
{
    resource::ImageHeader header;
    ResourceLoader::registerDecoder(new ImageStbDecoder({ "jpg" }, header));

    ResourceLoader::registerPath("../../../../");
    ResourceLoader::registerPath("../../../../../");
    ResourceLoader::registerPath("../../../../engine/");
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

           LOG_DEBUG("ImageFileLoader::load Shader [%s] is loaded", name.c_str());
           return static_cast<resource::Image*>(resource);
        }
    }

    LOG_WARNING("ImageFileLoader::load: File [%s] not found", name.c_str());
    return nullptr;
}

} //namespace resource
} //namespace v3d
