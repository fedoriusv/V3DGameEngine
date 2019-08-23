#include "ModelFileLoader.h"

#include "MeshAssimpDecoder.h"
#include "Scene/Model.h"

#include "Stream/FileLoader.h"
#include "Resource/ResourceLoaderManager.h"
#include "Renderer/Context.h"

namespace v3d
{
namespace resource
{

ModelFileLoader::ModelFileLoader(u32 flags) noexcept
{
#ifdef USE_ASSIMP
    scene::ModelHeader header;
    ResourceLoader::registerDecoder(new MeshAssimpDecoder({ "dae" }, header, flags));
#endif //USE_ASSIMP
    ResourceLoader::registerPath("../../../../");
    ResourceLoader::registerPath("../../../../../");
    ResourceLoader::registerPath("../../../../engine/");
    ResourceLoader::registerPathes(ResourceLoaderManager::getInstance()->getPathes());
}

ModelFileLoader::ModelFileLoader(const ResourceHeader* header, u32 flags) noexcept
{
#ifdef USE_ASSIMP
    scene::ModelHeader modelHeader = *static_cast<const scene::ModelHeader*>(header);
    ResourceLoader::registerDecoder(new MeshAssimpDecoder({ "dae" }, modelHeader, flags));
#endif //USE_ASSIMP
    ResourceLoader::registerPath("../../../../");
    ResourceLoader::registerPath("../../../../../");
    ResourceLoader::registerPath("../../../../engine/");
    ResourceLoader::registerPathes(ResourceLoaderManager::getInstance()->getPathes());
}

ModelFileLoader::~ModelFileLoader()
{
}

scene::Model* ModelFileLoader::load(const std::string & name, const std::string & alias)
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
               LOG_ERROR("ModelFileLoader: Streaming error read file [%s]", name.c_str());
               return nullptr;
           }

           if (!resource->load())
           {
               LOG_ERROR("ModelFileLoader: Streaming error read file [%s]", name.c_str());
               return nullptr;
           }

           LOG_INFO("ModelFileLoader::load Shader [%s] is loaded", name.c_str());
           return static_cast<scene::Model*>(resource);
        }
    }

    LOG_WARNING("ModelFileLoader::load: File [%s] decoder hasn't found", name.c_str());
    return nullptr;
}

} //namespace resource
} //namespace v3d
