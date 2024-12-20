#include "AssetSourceFileLoader.h"
#include "ResourceManager.h"
#include "AssetJSONDecoder.h"
#include "Stream/FileLoader.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace resource
{
AssetSourceFileLoader::AssetSourceFileLoader(u32 flags) noexcept
{
    ResourceDecoderRegistration::registerDecoder(V3D_NEW(AssetJSONDecoder, memory::MemoryLabel::MemorySystem)({ "json" }));
    ResourceLoader::registerPaths(ResourceManager::getInstance()->getPaths());
}

Resource* AssetSourceFileLoader::load(const std::string& name, const std::string& alias)
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
                LOG_WARNING("AssetSourceFileLoader::load: File [%s] decoder hasn't found", name.c_str());
                return nullptr;
            }

            Resource* resource = decoder->decode(file, nullptr, 0, name);

            stream::FileLoader::close(file);
            file = nullptr;

            if (!resource)
            {
                LOG_ERROR("AssetSourceFileLoader: Streaming error read file [%s]", name.c_str());
                return nullptr;
            }

            LOG_INFO("AssetSourceFileLoader::load: [%s] is loaded", name.c_str());
            return resource;
        }
    }

    LOG_WARNING("AssetSourceFileLoader::load: File [%s] hasn't found", name.c_str());
    return nullptr;
}

} //namespace resource
} //namespace v3d