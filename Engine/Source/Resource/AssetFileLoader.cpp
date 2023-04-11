#include "AssetFileLoader.h"
#include "ResourceLoaderManager.h"
#include "AssetDecoder.h"
#include "Stream/FileLoader.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace resource
{
AssetFileLoader::AssetFileLoader(AssetLoaderFlags flags) noexcept
{
    ResourceDecoderRegistration::registerDecoder(V3D_NEW(AssetDecoder, memory::MemoryLabel::MemorySystem)({ "v3dasset" }));

    ResourceLoader::registerRoot("");
    ResourceLoader::registerRoot("../../../../");

    ResourceLoader::registerPath("");
    ResourceLoader::registerPathes(ResourceLoaderManager::getInstance()->getPathes());
}

Resource* AssetFileLoader::load(const std::string& name, const std::string& alias)
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
                LOG_WARNING("AssetFileLoader::load: File [%s] decoder hasn't found", name.c_str());
                return nullptr;
            }

            Resource* resource = decoder->decode(file, name);

            file->close();
            V3D_DELETE(file, memory::MemoryLabel::MemoryResource);
            file = nullptr;

            if (!resource)
            {
                LOG_ERROR("AssetFileLoader: Streaming error read file [%s]", name.c_str());
                return nullptr;
            }

            LOG_INFO("AssetFileLoader::load: [%s] is loaded", name.c_str());
            return resource;
        }
    }

    LOG_WARNING("AssetFileLoader::load: File [%s] hasn't found", name.c_str());
    return nullptr;
}

} //namespace resource
} //namespace v3d