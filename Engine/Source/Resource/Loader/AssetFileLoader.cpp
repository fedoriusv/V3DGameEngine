#include "AssetFileLoader.h"

#include "Stream/FileLoader.h"
#include "Utils/Logger.h"

#include "Resource/ResourceManager.h"
#include "Resource/Decoder/AssetDecoder.h"

namespace v3d
{
namespace resource
{
AssetFileLoader::AssetFileLoader(AssetLoaderFlags flags) noexcept
    : m_flags(flags)
{
    ResourceDecoderRegistration::registerDecoder(V3D_NEW(AssetDecoder, memory::MemoryLabel::MemorySystem)({ "v3dasset" }));
    ResourceLoader::registerPaths(ResourceManager::getInstance()->getPaths());
}

Resource* AssetFileLoader::load(const std::string& name, const std::string& alias)
{
    for (std::string& root : m_roots)
    {
        for (std::string& path : m_paths)
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

            Resource* resource = decoder->decode(file, nullptr, m_flags, name);

            file->close();
            V3D_DELETE(file, memory::MemoryLabel::MemorySystem);
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