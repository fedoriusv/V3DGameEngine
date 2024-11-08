#include "ModelFileLoader.h"
#include "MeshAssimpDecoder.h"
#include "Renderer/Device.h"
#include "Stream/FileLoader.h"
#include "Resource/ResourceManager.h"

#include "Scene/Model.h"

namespace v3d
{
namespace resource
{

ModelFileLoader::ModelFileLoader(ModelLoaderFlags flags) noexcept
{
#ifdef USE_ASSIMP
    scene::ModelHeader header;
    ResourceDecoderRegistration::registerDecoder(V3D_NEW(MeshAssimpDecoder, memory::MemoryLabel::MemorySystem)({ "dae", "fbx" }, header, flags));
#endif //USE_ASSIMP

    ResourceLoader::registerPathes(ResourceManager::getInstance()->getPathes());
}

ModelFileLoader::ModelFileLoader(ResourceDecoder::Policy* policy, ModelLoaderFlags flags) noexcept
{
#ifdef USE_ASSIMP
    scene::ModelHeader modelHeader = *static_cast<const scene::ModelHeader*>(header);
    ResourceDecoderRegistration::registerDecoder(V3D_NEW(MeshAssimpDecoder, memory::MemoryLabel::MemorySystem)({ "dae", "fbx" }, modelHeader, flags));
#endif //USE_ASSIMP

    ResourceLoader::registerPathes(ResourceManager::getInstance()->getPathes());
}

scene::Model* ModelFileLoader::load(const std::string& name, const std::string& alias)
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
                LOG_WARNING("ModelFileLoader::load: File [%s] decoder hasn't found", name.c_str());
                return nullptr;
            }

            Resource* resource = decoder->decode(file, nullptr, 0, name);

            file->close();
            V3D_DELETE(file, memory::MemoryLabel::MemorySystem);
            file = nullptr;

            if (!resource)
            {
                LOG_ERROR("ModelFileLoader: Streaming error read file [%s]", name.c_str());
                return nullptr;
            }

            LOG_INFO("ModelFileLoader::load: [%s] is loaded", name.c_str());
            return static_cast<scene::Model*>(resource);
        }
    }

    LOG_WARNING("ModelFileLoader::load: File [%s] hasn't found", name.c_str());
    return nullptr;
}

} //namespace resource
} //namespace v3d
