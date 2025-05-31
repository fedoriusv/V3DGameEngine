#include "ModelFileLoader.h"

#include "Renderer/Device.h"
#include "Stream/FileLoader.h"

#include "Resource/ResourceManager.h"
#include "Resource/Model.h"
#include "Resource/Decoder/AssimpDecoder.h"

#define MODEL_FORMAT_DAE "dae"
#define MODEL_FORMAT_FBX "fbx"
#define MODEL_FORMAT_GLTF "gltf"

namespace v3d
{
namespace resource
{

ModelFileLoader::ModelFileLoader(ModelLoaderFlags flags) noexcept
    : m_policy()
    , m_flags(flags)
{
#ifdef USE_ASSIMP
    ResourceDecoderRegistration::registerDecoder(V3D_NEW(AssimpDecoder, memory::MemoryLabel::MemorySystem)({ MODEL_FORMAT_DAE, MODEL_FORMAT_FBX, MODEL_FORMAT_GLTF }));
#endif //USE_ASSIMP

    ResourceLoader::registerPaths(ResourceManager::getInstance()->getPaths());
}

ModelFileLoader::ModelFileLoader(const ModelFileLoader::ModelPolicy& policy, ModelLoaderFlags flags) noexcept
    : m_policy(policy)
    , m_flags(flags)
{
#ifdef USE_ASSIMP
    ResourceDecoderRegistration::registerDecoder(V3D_NEW(AssimpDecoder, memory::MemoryLabel::MemorySystem)({ MODEL_FORMAT_DAE, MODEL_FORMAT_FBX, MODEL_FORMAT_GLTF }));
#endif //USE_ASSIMP

    ResourceLoader::registerPaths(ResourceManager::getInstance()->getPaths());
}

ModelResource* ModelFileLoader::load(const std::string& name, const std::string& alias)
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
                LOG_WARNING("ModelFileLoader::load: File [%s] decoder hasn't found", name.c_str());
                return nullptr;
            }

            Resource* resource = decoder->decode(file, &m_policy, m_flags, name);

            stream::FileLoader::close(file);
            file = nullptr;

            if (!resource)
            {
                LOG_ERROR("ModelFileLoader: Streaming error read file [%s]", name.c_str());
                return nullptr;
            }

            LOG_INFO("ModelFileLoader::load: [%s] is loaded", name.c_str());
            return static_cast<ModelResource*>(resource);
        }
    }

    LOG_WARNING("ModelFileLoader::load: File [%s] hasn't found", name.c_str());
    return nullptr;
}

} //namespace resource
} //namespace v3d
