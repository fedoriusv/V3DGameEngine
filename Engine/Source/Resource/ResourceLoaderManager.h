#pragma once

#include "Common.h"
#include "Utils/Singleton.h"

namespace v3d
{
namespace renderer
{
    class Context;
}
namespace resource
{
    class Resource;

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    
    /**
    * ResourceLoaderManager
    */
    class ResourceLoaderManager : public utils::Singleton<ResourceLoaderManager>
    {
    public:

        ResourceLoaderManager() = default;

        template<class TResource, class TResourceLoader>
        TResource* loadFromFile(renderer::Context* context, std::string filename)
        {
            std::string innerName(filename);
            std::transform(filename.begin(), filename.end(), innerName.begin(), ::tolower);

            auto resourceIter = m_resources.emplace(std::make_pair(filename, nullptr));
            if (resourceIter.second)
            {
                TResourceLoader loader(context);
                Resource* res = loader.load(innerName);
                if (!res)
                {
                    m_resources.erase(innerName);
                    return nullptr;
                }
                
                resourceIter.first->second = res;
                return static_cast<TResource*>(res);
            }

            return static_cast<TResource*>(resourceIter.first->second);
        }

    private:

        std::map<std::string, Resource*> m_resources;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace resource
} //namespace v3d
